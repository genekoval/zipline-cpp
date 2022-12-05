#pragma once

#include "../codable.hpp"

#include <ios>
#include <optional>

namespace zipline {
    template <typename T, io::reader Reader>
    requires decodable<T, Reader>
    struct decoder<std::optional<T>, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::optional<T>> {
            const auto has_value = co_await zipline::decode<bool>(reader);

            TIMBER_TRACE(
                "decode optional: {}",
                has_value ? "has value" : "no value"
            );

            if (has_value) {
                co_return co_await zipline::decode<T>(reader);
            }

            co_return std::nullopt;
        }

    };

    template <typename T, io::writer Writer>
    requires encodable<T, Writer>
    struct encoder<std::optional<T>, Writer> {
        static auto encode(
            const std::optional<T>& optional,
            Writer& writer
        ) -> ext::task<> {
            const auto has_value = optional.has_value();
            co_await zipline::encode(has_value, writer);

            if (has_value) co_await zipline::encode(*optional, writer);
        }
    };
}
