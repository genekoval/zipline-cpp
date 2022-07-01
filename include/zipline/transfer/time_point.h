#pragma once

#include "base.h"

#include <chrono>

namespace zipline {
    template <typename Socket, typename Clock, typename Duration>
    class transfer<Socket, std::chrono::time_point<Clock, Duration>> {
        using duration = std::chrono::milliseconds;
        using time_point = std::chrono::time_point<Clock, Duration>;
        using underlying_type = int64_t;
    public:
        static auto read(Socket& socket) -> time_point {
            const auto millis = transfer<Socket, underlying_type>::read(socket);
            return time_point(duration(millis));
        }

        static auto write(Socket& socket, time_point tp) -> void {
            const auto millis = std::chrono::time_point_cast<duration>(tp)
                .time_since_epoch()
                .count();

            transfer<Socket, underlying_type>::write(socket, millis);
        }
    };
}
