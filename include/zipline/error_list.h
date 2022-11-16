#pragma once

#include <zipline/error.h>
#include <zipline/coder/coder.h>

#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <variant>

namespace zipline {
    using status_type = std::uint8_t;

    template <typename Socket, typename ...Errors>
    class error_list {
        using error = std::variant<std::monostate, Errors...>;
        using error_reader = auto (*)(Socket&) -> ext::task<error>;
        using error_types = std::tuple<Errors...>;

        template <std::size_t N>
        using type = typename std::tuple_element_t<N, error_types>;

        std::array<error_reader, sizeof...(Errors)> errors;
        std::unordered_map<std::type_index, status_type> codes;

        template <std::size_t ...I>
        auto initialize(std::index_sequence<I...>) -> void {
            ((errors[I] = [](Socket& sock) -> ext::task<error> {
                co_return co_await coder<Socket, type<I>>::decode(sock);
            }), ...);

            ((codes[std::type_index(typeid(type<I>))] = I), ...);
        }
    public:
        error_list() {
            initialize(std::index_sequence_for<Errors...>());
        }

        auto code(const zipline_error_base<Socket>& ex) const -> status_type {
            const auto result = codes.find(std::type_index(typeid(ex)));

            if (result == codes.end()) throw std::runtime_error(
                std::string("unregistered error type: ") +
                typeid(ex).name()
            );

            return result->second;
        }

        auto throw_error(Socket& sock, status_type index) const -> ext::task<> {
            if (index >= errors.size()) {
                throw std::runtime_error(fmt::format(
                    "unrecognized error code ({})",
                    index
                ));
            }

            std::visit(
                [](auto&& ex) { throw std::move(ex); },
                co_await errors[index](sock)
            );

            __builtin_unreachable();
        }

        constexpr auto size() const -> std::size_t {
            return errors.size();
        }
    };
}
