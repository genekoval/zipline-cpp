#pragma once

#include "error.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <ext/coroutine>
#include <span>
#include <timber/timber>

namespace zipline {
    template <typename Socket, std::size_t BufferSize>
    class buffered_base {
        std::array<std::byte, BufferSize> buffer;
    protected:
        Socket* socket = nullptr;
        std::size_t head = 0;
        std::size_t tail = 0;

        buffered_base() = default;

        explicit buffered_base(Socket& socket) : socket(&socket) {}

        constexpr auto capacity() -> std::size_t { return buffer.size(); }

        auto clear() -> void {
            head = 0;
            tail = 0;
        }

        auto empty() -> bool { return size() == 0; }

        auto front() -> std::byte* { return &buffer[head]; }

        auto size() -> std::size_t { return tail - head; }

        auto back() -> std::byte* { return &buffer[tail]; }
    };

    template <typename Socket, std::size_t BufferSize>
    class buffered_reader : public buffered_base<Socket, BufferSize> {
        auto fill_buffer() -> ext::task<> {
            this->clear();

            this->tail = co_await this->socket->read(
                this->front(),
                this->capacity()
            );

            if (this->empty()) throw eof();
        }

        auto read_bytes(std::byte* dest, std::size_t len) -> ext::task<> {
            auto remaining = len;

            while (remaining > 0) {
                if (this->empty()) co_await fill_buffer();

                const auto available = this->size();
                const auto bytes = std::min(remaining, available);

                std::memcpy(dest, this->front(), bytes);
                this->head += bytes;
                remaining -= bytes;

                if (remaining > 0) {
                    for (auto i = 0UL; i < bytes; ++i) ++dest;
                }
            }
        }
    public:
        buffered_reader() = default;

        explicit buffered_reader(Socket& socket) :
            buffered_base<Socket, BufferSize>(socket)
        {}

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>> {
            if (this->empty()) co_await fill_buffer();

            const auto available = this->size();
            const auto bytes = std::min(len, available);

            const auto result = std::span(this->front(), bytes);
            this->head += bytes;

            co_return result;
        }

        auto read(void* dest, std::size_t len) -> ext::task<> {
            co_await read_bytes(
                reinterpret_cast<std::byte*>(dest),
                len
            );
        }
    };

    template <typename Socket, std::size_t BufferSize>
    class buffered_writer : public buffered_base<Socket, BufferSize> {
        auto free() -> ext::task<std::size_t> {
            const auto available = this->capacity() - this->tail;
            if (available > 0) co_return available;

            co_await flush();
            co_return this->capacity();
        }

        auto write_bytes(const std::byte* src, std::size_t len) -> ext::task<> {
            auto remaining = len;

            while (remaining > 0) {
                const auto available = co_await free();
                const auto& bytes = std::min(remaining, available);

                std::memcpy(this->back(), src, bytes);
                this->tail += bytes;
                remaining -= bytes;

                if (remaining > 0) {
                    for (auto i = 0UL; i < bytes; ++i) ++src;
                }
            }
        }
    public:
        buffered_writer() = default;

        explicit buffered_writer(Socket& socket) :
            buffered_base<Socket, BufferSize>(socket)
        {}

        auto flush() -> ext::task<> {
            for (
                auto remaining = this->size();
                remaining > 0;
                remaining = this->size()
            ) {
                this->head +=
                    co_await this->socket->write(this->front(), remaining);
            }

            this->clear();
        }

        auto write(const void* src, std::size_t len) -> ext::task<> {
            co_await write_bytes(
                reinterpret_cast<const std::byte*>(src),
                len
            );
        }
    };

    template <typename Socket, std::size_t BufferSize>
    class buffered_socket {
        Socket socket;
        buffered_reader<Socket, BufferSize> reader;
        buffered_writer<Socket, BufferSize> writer;
    public:
        buffered_socket() = default;

        explicit buffered_socket(Socket&& socket) :
            socket(std::forward<Socket>(socket)),
            reader(this->socket),
            writer(this->socket)
        {}

        auto flush() -> ext::task<> { co_await writer.flush(); }

        auto inner() noexcept -> Socket& { return socket; }

        auto into_inner() noexcept -> Socket { return std::move(socket); }

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>> {
            co_return co_await reader.read(len);
        }

        auto read(void* dest, std::size_t len) -> ext::task<> {
            co_await reader.read(dest, len);
        }

        auto write(const void* src, std::size_t len) -> ext::task<> {
            co_await writer.write(src, len);
        }
    };

    template <typename Socket, std::size_t BufferSize>
    auto operator<<(
        std::ostream& os,
        const buffered_socket<Socket, BufferSize>& buffer
    ) -> std::ostream& {
        os << buffer.inner();
        return os;
    }
}
