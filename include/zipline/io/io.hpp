#pragma once

#include <ext/coroutine>
#include <span>

namespace zipline::io {
    template <typename T>
    concept reader = requires(T& t, void* dest, std::size_t len) {
        { t.fill_buffer() } -> std::same_as<ext::task<bool>>;
        { t.read(len) } -> std::same_as<ext::task<std::span<const std::byte>>>;
        { t.read(dest, len) } -> std::same_as<ext::task<>>;
    };

    template <typename T>
    concept writer = requires(T& t, const void* src, std::size_t len) {
        { t.await_write() } -> std::same_as<ext::task<>>;
        { t.flush() } -> std::same_as<ext::task<>>;
        { t.try_write(src, len) } -> std::convertible_to<std::size_t>;
        { t.write(src, len) } -> std::same_as<ext::task<>>;
    };

    struct abstract_reader {
        virtual auto fill_buffer() -> ext::task<bool> = 0;

        virtual auto read(std::size_t len) ->
            ext::task<std::span<const std::byte>> = 0;

        virtual auto read(void* dest, std::size_t len) -> ext::task<> = 0;
    };

    static_assert(reader<abstract_reader>);

    struct abstract_writer {
        virtual auto await_write() -> ext::task<> = 0;

        virtual auto flush() -> ext::task<> = 0;

        virtual auto try_write(
            const void* src,
            std::size_t len
        ) -> std::size_t = 0;

        virtual auto write(const void* src, std::size_t len) -> ext::task<> = 0;
    };

    static_assert(writer<abstract_writer>);

    struct abstract_io : abstract_reader, abstract_writer {};

    template <io::reader Reader>
    class abstract_reader_wrapper final : public io::abstract_reader {
        Reader& inner;
    public:
        abstract_reader_wrapper(Reader& inner) : inner(inner) {}

        auto fill_buffer() -> ext::task<bool> override {
            return inner.fill_buffer();
        }

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

        auto await_write() -> ext::task<> override {
            return inner.await_write();
        }

        auto flush() -> ext::task<> override {
            return inner.flush();
        }

        auto try_write(
            const void* src,
            std::size_t len
        ) -> std::size_t override {
            return inner.try_write(src, len);
        }

        auto write(
            const void* src,
            std::size_t len
        ) -> ext::task<> override {
            return inner.write(src, len);
        }
    };
}
