#pragma once

#include "../codable.hpp"

#include <timber/timber>

namespace zipline::detail {
    template <std::integral T, io::reader Reader>
    struct decoder {
        static auto decode(Reader& reader) -> ext::task<T> {
            auto result = static_cast<T>(0);
            co_await reader.read(&result, sizeof(T));

            TIMBER_TRACE(
                "decode {}int{}: {:L}",
                std::unsigned_integral<T> ? "u" : "",
                sizeof(T) * 8,
                result
            );

            co_return result;
        }
    };

    template <std::integral T, io::writer Writer>
    struct encoder {
        static auto encode(T t, Writer& writer) -> ext::task<> {
            TIMBER_TRACE(
                "encode {}int{}: {:L}",
                std::unsigned_integral<T> ? "u" : "",
                sizeof(T) * 8,
                t
            );

            co_await writer.write(&t, sizeof(T));
        }
    };
}

namespace zipline {
    template <io::reader Reader>
    struct decoder<std::int8_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::int8_t> {
            co_return co_await detail::decoder<std::int8_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::int8_t, Writer> {
        static auto encode(std::int8_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::int8_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::int16_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::int16_t> {
            co_return co_await detail::decoder<std::int16_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::int16_t, Writer> {
        static auto encode(std::int16_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::int16_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::int32_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::int32_t> {
            co_return co_await detail::decoder<std::int32_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::int32_t, Writer> {
        static auto encode(std::int32_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::int32_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::int64_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::int64_t> {
            co_return co_await detail::decoder<std::int64_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::int64_t, Writer> {
        static auto encode(std::int64_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::int64_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::uint8_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::uint8_t> {
            co_return co_await detail::decoder<std::uint8_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::uint8_t, Writer> {
        static auto encode(std::uint8_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::uint8_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::uint16_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::uint16_t> {
            co_return co_await detail::decoder<std::uint16_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::uint16_t, Writer> {
        static auto encode(std::uint16_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::uint16_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::uint32_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::uint32_t> {
            co_return co_await detail::decoder<std::uint32_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::uint32_t, Writer> {
        static auto encode(std::uint32_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::uint32_t, Writer>::encode(n, writer);
        }
    };

    template <io::reader Reader>
    struct decoder<std::uint64_t, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::uint64_t> {
            co_return co_await detail::decoder<std::uint64_t, Reader>::decode(
                reader
            );
        }
    };

    template <io::writer Writer>
    struct encoder<std::uint64_t, Writer> {
        static auto encode(std::uint64_t n, Writer& writer) -> ext::task<> {
            co_await detail::encoder<std::uint64_t, Writer>::encode(n, writer);
        }
    };
}
