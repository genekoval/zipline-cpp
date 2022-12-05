#pragma once

#include "../codable.hpp"

namespace zipline {
    template <typename T1, typename T2, io::reader Reader>
    requires decodable<T1, Reader> && decodable<T2, Reader>
    struct decoder<std::pair<T1, T2>, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::pair<T1, T2>> {
            auto t1 = co_await zipline::decode<T1>(reader);
            auto t2 = co_await zipline::decode<T2>(reader);

            co_return std::make_pair(std::move(t1), std::move(t2));
        }
    };

    template <typename T1, typename T2, io::writer Writer>
    requires encodable<T1, Writer> && encodable<T2, Writer>
    struct encoder<std::pair<T1, T2>, Writer> {
        static auto encode(
            const std::pair<T1, T2>& pair,
            Writer& writer
        ) -> ext::task<> {
            co_await zipline::encode(std::get<0>(pair), writer);
            co_await zipline::encode(std::get<1>(pair), writer);
        }
    };
}
