#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <span>
#include <timber/timber>

namespace zipline {
    template <typename Socket, std::size_t BufferSize>
    class buffered_base {
        std::array<std::byte, BufferSize> buffer;
    protected:
        Socket* socket;
        std::size_t head = 0;
        std::size_t tail = 0;

        buffered_base(Socket& socket) : socket(&socket) {}

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
        auto fill_buffer() -> void {
            this->clear();

            this->tail = this->socket->read(
                this->front(),
                this->capacity()
            );

            if (this->empty()) {
                throw std::runtime_error("Reader received EOF");
            }
        }

        auto read_bytes(std::byte* dest, std::size_t len) -> void {
            auto remaining = len;

            while (remaining > 0) {
                if (this->empty()) fill_buffer();

                const auto available = this->size();
                const auto& bytes = std::min(remaining, available);

                std::memcpy(dest, this->front(), bytes);
                this->head += bytes;
                remaining -= bytes;

                if (remaining > 0) {
                    for (auto i = 0UL; i < bytes; ++i) ++dest;
                }
            }
        }
    public:
        buffered_reader(Socket& socket) :
            buffered_base<Socket, BufferSize>(socket)
        {}

        auto read(std::size_t len) -> std::span<const std::byte> {
            if (this->empty()) fill_buffer();

            const auto available = this->size();
            const auto& bytes = std::min(len, available);

            const auto result = std::span(this->front(), bytes);
            this->head += bytes;

            return result;
        }

        auto read(void* dest, std::size_t len) -> void {
            read_bytes(
                reinterpret_cast<std::byte*>(dest),
                len
            );
        }
    };

    template <typename Socket, std::size_t BufferSize>
    class buffered_writer : public buffered_base<Socket, BufferSize> {
        auto free() -> std::size_t {
            const auto available = this->capacity() - this->tail;
            if (available > 0) return available;

            flush();
            return this->capacity();
        }

        auto write_bytes(const std::byte* src, std::size_t len) -> void {
            auto remaining = len;

            while (remaining > 0) {
                const auto available = free();
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
        buffered_writer(Socket& socket) :
            buffered_base<Socket, BufferSize>(socket)
        {}

        ~buffered_writer() {
            try {
                flush();
            }
            catch (const std::exception& ex) {
                ERROR()
                    << "Write operation failed with "
                    << this->size() << " bytes unsent: "
                    << ex.what();
            }
        }

        auto flush() -> void {
            for (
                auto remaining = this->size();
                remaining > 0;
                remaining = this->size()
            ) {
                this->head += this->socket->write(this->front(), remaining);
            }

            this->clear();
        }

        auto write(const void* src, std::size_t len) -> void {
            write_bytes(
                reinterpret_cast<const std::byte*>(src),
                len
            );
        }
    };

    template <typename Socket, std::size_t BufferSize>
    class buffered_socket {
        Socket sock;
        buffered_reader<Socket, BufferSize> reader;
        buffered_writer<Socket, BufferSize> writer;
    public:
        buffered_socket(Socket&& socket) :
            sock(std::move(socket)),
            reader(sock),
            writer(sock)
        {}

        buffered_socket(buffered_socket&& other) noexcept :
            sock(std::move(other.sock)),
            reader(sock),
            writer(sock)
        {}

        auto operator=(
            buffered_socket&& other
        ) noexcept -> buffered_socket& {
            sock = std::move(other.sock);
            reader = decltype(reader)(sock);
            writer = decltype(writer)(sock);
        }

        auto flush() -> void { writer.flush(); }

        auto read(std::size_t len) -> std::span<const std::byte> {
            return reader.read(len);
        }

        auto read(void* dest, std::size_t len) -> void {
            reader.read(dest, len);
        }

        auto socket() const -> const Socket& { return sock; }

        auto write(const void* src, std::size_t len) -> void {
            writer.write(src, len);
        }
    };

    template <typename Socket, std::size_t BufferSize>
    auto operator<<(
        std::ostream& os,
        const buffered_socket<Socket, BufferSize>& buffer
    ) -> std::ostream& {
        os << buffer.socket();
        return os;
    }
}
