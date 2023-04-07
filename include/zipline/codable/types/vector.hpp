#pragma once

#include "../codable.hpp"

#include <vector>
#include <timber/timber>

namespace zipline {
    template <typename T, io::reader Reader>
    requires decodable<T, Reader>
    struct decoder<std::vector<T>, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::vector<T>> {
            const auto size = co_await zipline::decode<std::size_t>(reader);

            TIMBER_TRACE("decode vector({:L})", size);

            auto vector = std::vector<T>();
            vector.reserve(size);

            for (std::size_t i = 0; i < size; ++i) {
                TIMBER_TRACE("decode vector[{}]", i);
                vector.push_back(co_await zipline::decode<T>(reader));
            }

            co_return vector;
        }
    };

    template <typename T, io::writer Writer>
    requires encodable<T, Writer>
    struct encoder<std::vector<T>, Writer> {
        static auto encode(
            const std::vector<T>& vector,
            Writer& writer
        ) -> ext::task<> {
            co_await zipline::encode<std::span<const T>>(vector, writer);
        }
    };
}
