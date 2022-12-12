#pragma once

#include <ext/coroutine>
#include <span>

namespace zipline::io {
    template <typename T>
    concept source = requires(T& t, void* dest, std::size_t len) {
        { t.read(dest, len) } -> std::same_as<ext::task<std::size_t>>;
    };

    template <typename T>
    concept sink = requires(T& t, const void* data, std::size_t len) {
        { t.write(data, len) } -> std::same_as<ext::task<std::size_t>>;
    };

    template <typename T>
    concept reader = requires(T& t, void* dest, std::size_t len) {
        { t.read(len) }-> std::same_as<ext::task<std::span<const std::byte>>>;
        { t.read(dest, len) } -> std::same_as<ext::task<>>;
    };

    template <typename T>
    concept writer = requires(T& t, const void* src, std::size_t len) {
        { t.flush() } -> std::same_as<ext::task<>>;
        { t.write(src, len) } -> std::same_as<ext::task<>>;
    };

    struct abstract_reader {
        virtual auto read(std::size_t len) ->
            ext::task<std::span<const std::byte>> = 0;

        virtual auto read(void* dest, std::size_t len) -> ext::task<> = 0;
    };

    static_assert(reader<abstract_reader>);

    struct abstract_writer {
        virtual auto flush() -> ext::task<> = 0;

        virtual auto write(const void* src, std::size_t len) -> ext::task<> = 0;
    };

    static_assert(writer<abstract_writer>);

    struct abstract_io : abstract_reader, abstract_writer {};

    template <io::reader Reader>
    class abstract_reader_wrapper final : public io::abstract_reader {
        Reader& inner;
    public:
        abstract_reader_wrapper(Reader& inner) : inner(inner) {}

        auto read(
            std::size_t len
        ) -> ext::task<std::span<const std::byte>> override {
            return inner.read(len);
        }

        auto read(void* dest, std::size_t len) -> ext::task<> override {
            return inner.read(dest, len);
        }
    };

    template <io::writer Writer>
    class abstract_writer_wrapper final : public io::abstract_writer {
        Writer& inner;
    public:
        abstract_writer_wrapper(Writer& inner) : inner(inner) {}

        auto flush() -> ext::task<> override {
            return inner.flush();
        }

        auto write(
            const void* src,
            std::size_t len
        ) -> ext::task<> override {
            return inner.write(src, len);
        }
    };
}
