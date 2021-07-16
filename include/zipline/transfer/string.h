#pragma once

#include "base.h"

#include <string>

namespace zipline {
    inline auto log_string(std::string_view string) -> void {
        const auto subsize = std::min(string.size(), 64ul);
        const auto substr = string.substr(0, subsize);

        TRACE()
            << "string: "
            << substr
            << (
                subsize < string.size() ?
                    " ...(" +
                    std::to_string(string.size() - subsize) +
                    " more)..." : ""
            );
    }

    template <typename Socket>
    struct transfer<Socket, std::string> {
        static auto read(Socket& socket) -> std::string {
            const auto string = read_array<Socket, std::string>(socket, '\0');

            if (timber::reporting_level >= timber::level::trace) {
                TRACE() << "read string";
                log_string(string);
            }

            return string;
        }

        static auto write(Socket& socket, const std::string& string) -> void {
            write_array(socket, string);

            if (timber::reporting_level >= timber::level::trace) {
                TRACE() << "write string";
                log_string(string);
            }
        }
    };

    template <typename Socket>
    struct transfer<Socket, std::string_view> {
        static auto read(Socket& socket) -> std::string_view {
            ERROR() << "string_view does not support reading";
            throw unsupported_transfer_type();
        }

        static auto write(
            Socket& socket,
            const std::string_view& string
        ) -> void {
            write_array(socket, string);

            if (timber::reporting_level >= timber::level::trace) {
                TRACE() << "write string_view";
                log_string(string);
            }
        }
    };
}
