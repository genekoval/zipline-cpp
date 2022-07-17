#pragma once

#include "base.h"

#include <chrono>

namespace zipline {
    template <typename Socket, typename Clock, typename Duration>
    class transfer<Socket, std::chrono::time_point<Clock, Duration>> {
        using time_point = std::chrono::time_point<Clock, Duration>;
        using underlying_type = int64_t;
    public:
        static auto read(Socket& socket) -> time_point {
            const auto value = transfer<Socket, underlying_type>::read(socket);
            TIMBER_TRACE("read time point: {}", value);
            return time_point(Duration(value));
        }

        static auto write(Socket& socket, time_point tp) -> void {
            const auto value = tp.time_since_epoch().count();
            TIMBER_TRACE("write time point: {}", value);
            transfer<Socket, underlying_type>::write(socket, value);
        }
    };
}
