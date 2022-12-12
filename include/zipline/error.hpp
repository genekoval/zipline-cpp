#pragma once

#include "codable/codable"

#include <stdexcept>

namespace zipline {
    using status_type = std::uint8_t;

    struct eof : std::exception {
        auto what() const noexcept -> const char* override;
    };

    struct insufficient_space : std::exception {
        auto what() const noexcept -> const char* override;
    };

    struct internal_error : std::exception {
        auto what() const noexcept -> const char* override;
    };

    struct unknown_code : std::runtime_error {
        unknown_code(status_type status);
    };

    class zipline_error : public std::runtime_error {
        template <typename... Args>
        static auto make_what(
            std::string_view format_string,
            Args&&... args
        ) -> std::string {
            if constexpr (sizeof...(args) == 0) {
                return std::string(format_string);
            }

            return fmt::format(
                fmt::runtime(format_string),
                std::forward<Args>(args)...
            );
        }
    public:
        template <typename... Args>
        zipline_error(std::string_view format_string, Args&&... args) :
            std::runtime_error(make_what(
                format_string,
                std::forward<Args>(args)...
            ))
        {}

        virtual ~zipline_error() = default;

        virtual auto encode(io::abstract_writer& writer) const -> ext::task<>;
    };

    template <typename T>
    requires std::is_base_of_v<zipline_error, T>
    struct decoder<T, io::abstract_reader> {
        static auto decode(
            io::abstract_reader& reader
        ) -> ext::task<T> {
            const auto message = co_await zipline::decode<std::string>(reader);
            co_return T{message};
        }
    };

    static_assert(decodable<zipline_error>);
}
