#pragma once

#include "../codable.hpp"

namespace zipline {
    template <typename T, io::writer Writer>
    requires encodable<T, Writer>
    struct encoder<std::span<const T>, Writer> {
        static auto encode(
            std::span<const T> span,
            Writer& writer
        ) -> ext::task<> {
            co_await zipline::encode(span.size(), writer);

            for (const auto& item : span) {
                co_await zipline::encode(item, writer);
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
            co_await zipline::encode(bytes.size(), writer);

            co_await writer.write(bytes.data(), size);
        }
    };
}
