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
    struct coder<Socket, std::string> {
        static auto decode(Socket& socket) -> ext::task<std::string> {
            const auto string =
                co_await decode_array<Socket, std::string>(socket, '\0');

            if (timber::reporting_level >= timber::level::trace) {
                TIMBER_TRACE("read string");
                log_string(string);
            }

            co_return string;
        }

        static auto encode(
            Socket& socket,
            const std::string& string
        ) -> ext::task<> {
            co_await encode_array(socket, string);

            if (timber::reporting_level >= timber::level::trace) {
                TIMBER_TRACE("write string");
                log_string(string);
            }
        }
    };

    template <typename Socket>
    struct coder<Socket, std::string_view> {
        static auto encode(
            Socket& socket,
            const std::string_view& string
        ) -> ext::task<> {
            co_await encode_array(socket, string);

            if (timber::reporting_level >= timber::level::trace) {
                TIMBER_TRACE("write string_view");
                log_string(string);
            }
        }
    };
}
