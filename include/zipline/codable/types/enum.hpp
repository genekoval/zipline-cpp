#pragma once

#include "../codable.hpp"

namespace zipline {
    template <typename T, io::reader Reader>
    requires std::is_enum_v<T> && decodable<std::underlying_type_t<T>, Reader>
    struct decoder<T, Reader> {
        static auto decode(Reader& reader) -> ext::task<T> {
            using underlying = std::underlying_type_t<T>;

            const auto value = co_await zipline::decode<underlying>(reader);
            co_return static_cast<T>(value);
        }
    };

    template <typename T, io::writer Writer>
    requires std::is_enum_v<T> && encodable<std::underlying_type_t<T>, Writer>
    struct encoder<T, Writer> {
        static auto encode(T t, Writer& writer) -> ext::task<> {
            using underlying = std::underlying_type_t<T>;

            co_await zipline::encode<underlying>(
                static_cast<underlying>(t),
                writer
            );
        }
    };
}
