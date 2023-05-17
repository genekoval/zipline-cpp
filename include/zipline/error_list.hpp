#pragma once

#include "error.hpp"
#include "codable/codable"

#include <optional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace zipline {
    struct error_codes final {
        using map_type = std::unordered_map<std::type_index, status_type>;
    private:
        map_type codes;
    public:
        error_codes(map_type&& codes);

        error_codes(const error_codes&) = delete;

        error_codes(error_codes&&) = delete;

        auto operator=(const error_codes&) -> error_codes& = delete;

        auto operator=(error_codes&&) -> error_codes& = delete;

        auto code(const std::exception& ex) const -> std::optional<status_type>;
    };

    struct error_thrower final {
        using thrower = auto (*)(io::abstract_reader&) -> ext::task<>;
        using thrower_list = std::unique_ptr<thrower[]>;
    private:
        thrower_list throwers;
        std::size_t size = 0;
    public:
        error_thrower(thrower_list&& throwers, std::size_t size);

        error_thrower(const error_thrower&) = delete;

        error_thrower(error_thrower&&) = delete;

        auto operator=(const error_thrower&) -> error_thrower& = delete;

        auto operator=(error_thrower&&) -> error_thrower& = delete;

        auto throw_error(
            status_type status,
            io::abstract_reader& reader
        ) const -> ext::task<>;
    };

    template <typename... Errors>
    requires
        (std::derived_from<Errors, zipline_error> && ...) ||
        (sizeof...(Errors) == 0)
    class error_list {
        using errors = std::tuple<Errors...>;

        template <std::size_t N>
        using error_type = typename std::tuple_element_t<N, errors>;

        template <std::size_t... I>
        static auto make_codes(std::index_sequence<I...>) -> error_codes {
            auto map = error_codes::map_type();

            ((map[std::type_index(typeid(error_type<I>))] = I), ...);

            return error_codes(std::move(map));
        }

        template <std::size_t... I>
        static auto make_thrower(
            std::index_sequence<I...>
        ) -> error_thrower {
            using thrower = typename error_thrower::thrower;
            using thrower_list = typename error_thrower::thrower_list;

            auto throwers = thrower_list(new thrower[size()]);

            ((throwers[I] = [](io::abstract_reader& reader) -> ext::task<> {
                throw co_await zipline::decode<error_type<I>>(reader);
            }), ...);

            return error_thrower(std::move(throwers), size());
        }
    public:
        error_list() = delete;

        static auto codes() -> const error_codes& {
            static const auto instance =
                make_codes(std::index_sequence_for<Errors...>());

            return instance;
        }

        static auto thrower() -> const error_thrower& {
            static const auto instance =
                make_thrower(std::index_sequence_for<Errors...>());

            return instance;
        }

        static constexpr auto size() noexcept -> std::size_t {
            return sizeof...(Errors);
        }
    };
}
