#pragma once

#include "../codable.hpp"

#include <timber/timber>

namespace zipline {
    template <io::reader Reader>
    struct decoder<bool, Reader> {
        static auto decode(Reader& reader) -> ext::task<bool> {
            const auto value = co_await zipline::decode<std::uint8_t>(reader);

            if (value == 1) {
                TIMBER_TRACE("decode boolean: true");
                co_return true;
            }

            if (value == 0) {
                TIMBER_TRACE("decode boolean: false");
                co_return false;
            }

            throw std::system_error(
                EINVAL,
                std::generic_category(),
                fmt::format("invalid bit pattern for boolean type: {}", value)
            );
        }
    };

    template <io::writer Writer>
    struct encoder<bool, Writer> {
        static auto encode(bool b, Writer& writer) -> ext::task<> {
            TIMBER_TRACE("encode boolean: {}", b);

            std::uint8_t value = b ? 1 : 0;
            co_await zipline::encode(value, writer);
        }
    };
}
