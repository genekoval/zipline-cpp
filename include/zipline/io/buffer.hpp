#pragma once

#include "null.hpp"

#include "../error.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <ext/coroutine>
#include <span>
#include <timber/timber>

namespace zipline::io {
    template <std::size_t N>
    class array_buffer final {
        std::array<std::byte, N> buffer;
        std::size_t head = 0;
        std::size_t tail = 0;

        auto read_bytes(std::byte* dest, std::size_t len) -> ext::task<> {
            if (len > size()) throw eof();

            std::memcpy(dest, front(), len);
            bytes_read(len);

            co_return;
        }

        auto write_bytes(const std::byte* src, std::size_t len) -> ext::task<> {
            if (len > available()) throw insufficient_space();

            std::memcpy(back(), src, len);
            tail += len;

            co_return;
        }
    public:
        constexpr auto bytes_read(std::size_t n) -> void {
            head += n;
            if (head == tail) clear();
        }

        constexpr auto bytes_written(std::size_t n) -> void { tail += n; }

        constexpr auto available() const noexcept -> std::size_t {
            return N - tail;
        }

        constexpr auto back() -> std::byte* { return &buffer[tail]; }

        constexpr auto capacity() -> std::size_t { return N; }

        constexpr auto clear() -> void {
            head = 0;
            tail = 0;
        }

        constexpr auto empty() -> bool { return size() == 0; }

        auto flush() -> ext::task<> { co_return; }

        constexpr auto front() -> std::byte* { return &buffer[head]; }

        constexpr auto full() const noexcept -> bool {
            return available() == 0;
        }

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>> {
            const auto bytes = std::min(size(), len);

            const auto result = std::span<const std::byte>(front(), bytes);
            bytes_read(bytes);

            co_return result;
        }

        auto read(void* dest, std::size_t len) -> ext::task<> {
            return read_bytes(reinterpret_cast<std::byte*>(dest), len);
        }

        constexpr auto size() -> std::size_t { return tail - head; }

        auto write(const void* src, std::size_t len) -> ext::task<> {
            return write_bytes(reinterpret_cast<const std::byte*>(src), len);
        }
    };

    static_assert(reader<array_buffer<1>>);
    static_assert(writer<array_buffer<1>>);

    template <source Inner, std::size_t N>
    class buffered_reader final {
        array_buffer<N> buffer;

        auto fill_buffer() -> ext::task<> {
            const auto bytes = co_await inner.read(
                buffer.front(),
                buffer.available()
            );

            buffer.bytes_written(bytes);

            if (buffer.empty()) throw eof();
        }

        auto read_bytes(std::byte* dest, std::size_t len) -> ext::task<> {
            auto remaining = len;

            if (len >= buffer.capacity()) {
                if (buffer.size() > 0) {
                    std::memcpy(dest, buffer.front(), buffer.size());
                    buffer.bytes_read(buffer.size());
                    remaining -= buffer.size();
                }

                while (remaining > 0) {
                    const auto bytes = co_await inner.read(dest, remaining);
                    if (bytes == 0) throw eof();

                    remaining -= bytes;
                    dest += bytes;
                }

                co_return;
            }

            while (remaining > 0) {
                if (buffer.empty()) co_await fill_buffer();

                const auto bytes = std::min(remaining, buffer.size());

                std::memcpy(dest, buffer.front(), bytes);
                buffer.bytes_read(bytes);
                remaining -= bytes;
                dest += bytes;
            }
        }
    public:
        Inner& inner;

        explicit buffered_reader(Inner& inner) : inner(inner) {}

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>> {
            if (buffer.size() < len) co_await fill_buffer();
            co_return co_await buffer.read(len);
        }

        auto read(void* dest, std::size_t len) -> ext::task<> {
            return read_bytes(reinterpret_cast<std::byte*>(dest), len);
        }
    };

    static_assert(reader<buffered_reader<null, 1>>);

    template <sink Inner, std::size_t N>
    class buffered_writer final {
        array_buffer<N> buffer;

        auto free() -> ext::task<std::size_t> {
            const auto available = this->capacity() - this->tail;
            if (available > 0) co_return available;

            co_await flush();
            co_return this->capacity();
        }

        auto write_bytes(const std::byte* src, std::size_t len) -> ext::task<> {
            auto remaining = len;

            if (len >= buffer.capacity()) {
                remaining -= buffer.size();
                co_await flush();

                while (remaining > 0) {
                    const auto bytes = co_await inner.write(src, remaining);
                    remaining -= bytes;
                    src += bytes;
                }

                co_return;
            }

            while (remaining > 0) {
                if (buffer.full()) co_await flush();

                const auto bytes = std::min(remaining, buffer.available());

                std::memcpy(buffer.back(), src, bytes);
                buffer.bytes_written(bytes);
                remaining -= bytes;
                src += bytes;
            }
        }
    public:
        Inner& inner;

        explicit buffered_writer(Inner& inner) : inner(inner) {}

        auto flush() -> ext::task<> {
            while (!buffer.empty()) {
                const auto bytes = co_await inner.write(
                    buffer.front(),
                    buffer.size()
                );

                buffer.bytes_read(bytes);
            }
        }

        auto write(const void* src, std::size_t len) -> ext::task<> {
            return write_bytes(reinterpret_cast<const std::byte*>(src), len);
        }
    };

    static_assert(writer<buffered_writer<null, 1>>);

    template <typename Inner, std::size_t BufferSize>
    requires source<Inner> && sink<Inner>
    struct buffered {
        Inner inner;
    private:
        buffered_reader<Inner, BufferSize> reader;
        buffered_writer<Inner, BufferSize> writer;
    public:
        template <typename... Args>
        explicit buffered(Args&&... args) :
            inner(std::forward<Args>(args)...),
            reader(inner),
            writer(inner)
        {}

        buffered(const buffered&) = delete;

        buffered(buffered&& other) :
            inner(std::move(other.inner)),
            reader(inner),
            writer(inner)
        {}

        auto operator=(const buffered&) -> buffered& = delete;

        auto operator=(buffered&& other) -> buffered& {
            inner = std::move(other.inner);

            reader = std::move(other.reader);
            reader.inner = inner;

            writer = std::move(other.writer);
            writer.inner = inner;
        }

        auto flush() -> ext::task<> { co_await writer.flush(); }

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>> {
            return reader.read(len);
        }

        auto read(void* dest, std::size_t len) -> ext::task<> {
            return reader.read(dest, len);
        }

        auto write(const void* src, std::size_t len) -> ext::task<> {
            return writer.write(src, len);
        }
    };

    static_assert(reader<buffered<null, 1>>);
    static_assert(writer<buffered<null, 1>>);
}
