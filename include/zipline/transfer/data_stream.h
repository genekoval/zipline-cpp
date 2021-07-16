#pragma once

#include "base.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>

namespace zipline {
    template <typename Socket>
    class data_stream {
        std::span<const std::byte> chunk;
        Socket* socket = nullptr;
        std::optional<std::size_t> stream_size;
    public:
        data_stream() = default;

        data_stream(Socket& socket) : socket(&socket) {}

        template <typename Callable>
        auto read(Callable pipe) -> void {
            auto remaining = size();

            TRACE()
                << *socket << " reading data stream: " << remaining << " bytes";

            pipe(peek());
            remaining -= chunk.size();

            while (remaining > 0) {
                chunk = socket->read(remaining);
                remaining -= chunk.size();
                pipe(chunk);
            }
        }

        auto peek() -> std::span<const std::byte> {
            if (!chunk.data()) chunk = socket->read(size());
            return chunk;
        }

        auto size() -> std::size_t {
            if (!stream_size) {
                stream_size = read_size(*socket);
            }

            return *stream_size;
        }
    };

    template <typename Socket>
    struct transfer<Socket, data_stream<Socket>> {
        static auto read(Socket& socket) -> data_stream<Socket> {
            return data_stream<Socket>(socket);
        }

        static auto write(Socket& socket) -> void {
            ERROR() << "data_stream does not support writing";
            throw unsupported_transfer_type();
        }
    };
}
