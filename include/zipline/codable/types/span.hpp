#pragma once

#include "../codable.hpp"

#include <timber/timber>

namespace zipline {
    template <typename T, io::writer Writer>
    requires encodable<T, Writer>
    struct encoder<std::span<const T>, Writer> {
        static auto encode(
            std::span<const T> span,
            Writer& writer
        ) -> ext::task<> {
            const auto size = span.size();
            TIMBER_TRACE("encode span({:L})", size);

            co_await zipline::encode(span.size(), writer);

            for (std::size_t i = 0; i < size; ++i) {
                TIMBER_TRACE("encode span[{}]", i);
                co_await zipline::encode(span[i], writer);
            }
        }
    };

    template <io::writer Writer>
    struct encoder<std::span<const std::byte>, Writer> {
        static auto encode(
            std::span<const std::byte> bytes,
            Writer& writer
        ) -> ext::task<> {
            const auto size = bytes.size();
            TIMBER_TRACE("encode bytes({:L})", size);

            co_await zipline::encode(size, writer);
            co_await writer.write(bytes.data(), size);
        }
    };
}
