#pragma once

#include "base.h"

#include <chrono>

namespace zipline {
    template <typename Socket, typename Clock, typename Duration>
    class coder<Socket, std::chrono::time_point<Clock, Duration>> {
        using time_point = std::chrono::time_point<Clock, Duration>;
        using underlying_type = int64_t;
    public:
        static auto decode(Socket& socket) -> ext::task<time_point> {
            const auto value =
                co_await coder<Socket, underlying_type>::decode(socket);

            TIMBER_TRACE("read time point: {}", value);

            co_return time_point(Duration(value));
        }

        static auto encode(Socket& socket, time_point tp) -> ext::task<> {
            const auto value = tp.time_since_epoch().count();

            TIMBER_TRACE("write time point: {}", value);

            co_await coder<Socket, underlying_type>::encode(socket, value);
        }
    };
}
