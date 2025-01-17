#pragma once

#include "../codable.hpp"

#include <chrono>
#include <fmt/chrono.h>
#include <timber/timber>

namespace zipline {
    template <typename Clock, typename Duration, io::reader Reader>
    struct decoder<std::chrono::time_point<Clock, Duration>, Reader> {
        using time_point = std::chrono::time_point<Clock, Duration>;

        static auto decode(Reader& reader) -> ext::task<time_point> {
            const auto value = co_await zipline::decode<std::int64_t>(reader);
            const auto result = time_point(Duration(value));

            TIMBER_TRACE("decode time point: {}", result);

            co_return result;
        }
    };

    template <typename Clock, typename Duration, io::writer Writer>
    struct encoder<std::chrono::time_point<Clock, Duration>, Writer> {
        using time_point = std::chrono::time_point<Clock, Duration>;

        static auto encode(time_point tp, Writer& writer) -> ext::task<> {
            TIMBER_TRACE("encode time point: {}", tp);

            const auto value = tp.time_since_epoch().count();
            co_await zipline::encode<std::int64_t>(value, writer);
        }
    };
}
