#pragma once

#include "../codable.hpp"

#include <timber/timber>

namespace zipline {
    template <io::reader Reader>
    struct decoder<std::string, Reader> {
        static auto decode(Reader& reader) -> ext::task<std::string> {
            const auto size = co_await zipline::decode<std::size_t>(reader);

            auto string = std::string(size, '\0');
            co_await reader.read(string.data(), size);

            TIMBER_TRACE("decode string({:L}): {}", size, string);

            co_return string;
        }
    };

    template <io::writer Writer>
    struct encoder<std::string, Writer> {
        static auto encode(const std::string& string, Writer& writer)
            -> ext::task<> {
            co_await zipline::encode<std::string_view>(string, writer);
        }
    };

    template <io::writer Writer>
    struct encoder<std::string_view, Writer> {
        static auto encode(std::string_view string, Writer& writer)
            -> ext::task<> {
            const auto size = string.size();
            TIMBER_TRACE("encode string({:L}): {}", size, string);

            co_await zipline::encode(size, writer);

            co_await writer.write(string.data(), size);
        }
    };

    template <io::writer Writer>
    struct encoder<const char*, Writer> {
        static auto encode(const char* str, Writer& writer) -> ext::task<> {
            co_await zipline::encode<std::string_view>(str, writer);
        }
    };
}
