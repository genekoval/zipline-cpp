#pragma once

#include <zipline/io/io.hpp>

namespace zipline {
    template <typename T, typename Reader>
    struct decoder {};

    template <typename T, typename Writer>
    struct encoder {};

    template <typename T, typename Reader = io::abstract_reader>
    concept decodable = io::reader<Reader> && requires(Reader& reader) {
        { decoder<std::remove_cvref_t<T>, Reader>::decode(reader) } ->
            std::same_as<ext::task<std::remove_cvref_t<T>>>;
    };

    template <typename T, typename Writer = io::abstract_writer>
    concept encodable =
        io::writer<Writer> &&
        requires(const T& t, Writer& writer) {
            { encoder<std::remove_cvref_t<T>, Writer>::encode(t, writer) } ->
                std::same_as<ext::task<>>;
        };

    template <typename T, typename IO = io::abstract_io>
    concept codable = decodable<T, IO> && encodable<T, IO>;

    template <typename T, io::reader Reader>
    requires decodable<T, Reader>
    auto decode(Reader& reader) -> ext::task<std::remove_cvref_t<T>> {
        return decoder<std::remove_cvref_t<T>, Reader>::decode(reader);
    }

    template <typename T, io::writer Writer>
    requires encodable<T, Writer>
    auto encode(const T& t, Writer& writer) -> ext::task<> {
        return encoder<std::remove_cvref_t<T>, Writer>::encode(t, writer);
    }
}
