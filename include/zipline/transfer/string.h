#pragma once

#include "base.h"

#include <string>

namespace zipline {
    inline auto log_string(std::string_view string) -> void {
        const auto subsize = std::min(string.size(), 64ul);
        const auto substr = string.substr(0, subsize);

        TIMBER_TRACE("string: {}{}",
            substr,
            subsize < string.size() ?
                fmt::format("...({} more)...", string.size() - subsize) : ""
        );
    }

    template <typename Socket>
    struct transfer<Socket, std::string> {
        static auto read(Socket& socket) -> std::string {
            const auto string = read_array<Socket, std::string>(socket, '\0');

            if (timber::reporting_level >= timber::level::trace) {
                TIMBER_TRACE("read string");
                log_string(string);
            }

            return string;
        }

        static auto write(Socket& socket, const std::string& string) -> void {
            write_array(socket, string);

            if (timber::reporting_level >= timber::level::trace) {
                TIMBER_TRACE("write string");
                log_string(string);
            }
        }
    };

    template <typename Socket>
    struct transfer<Socket, std::string_view> {
        static auto read(Socket& socket) -> std::string_view {
            TIMBER_ERROR("string_view does not support reading");
            throw unsupported_transfer_type();
        }

        static auto write(
            Socket& socket,
            const std::string_view& string
        ) -> void {
            write_array(socket, string);

            if (timber::reporting_level >= timber::level::trace) {
                TIMBER_TRACE("write string_view");
                log_string(string);
            }
        }
    };
}
