#pragma once

#include "../codable.hpp"

namespace zipline {
    template <typename Reader>
    struct decoder<std::size_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::size_t> {
            co_return co_await zipline::decode<std::uint64_t>(reader);
        }
    };

    template <typename Writer>
    struct encoder<std::size_t, Writer> {
        static auto encode(Writer& writer, std::size_t size)
            -> ext::task<std::size_t> {
            co_await zipline::encode<std::uint64_t>(size, writer);
        }
    };
}
