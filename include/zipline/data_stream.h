#pragma once

#include <zipline/transfer.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <span>

namespace zipline {
    template <typename Socket>
    class data_stream {
        using size_type = std::size_t;

        std::span<const std::byte> chunk;
        Socket* sock;
        std::optional<size_type> stream_size;
    public:
        data_stream(Socket& sock) : sock(&sock) {}

        template <typename Callable>
        auto read(Callable pipe) -> void {
            auto remaining = size();

            DEBUG()
                << *sock << " reading data stream: " << remaining << " bytes";

            pipe(peek());
            remaining -= chunk.size();

            while (remaining > 0) {
                chunk = sock->read(remaining);
                remaining -= chunk.size();
                pipe(chunk);
            }
        }

        auto peek() -> std::span<const std::byte> {
            if (!chunk.data()) chunk = sock->read(size());
            return chunk;
        }

        auto size() -> size_type {
            if (!stream_size) {
                stream_size = transfer<Socket, size_type>::read(*sock);
            }

            return *stream_size;
        }
    };

    template <typename Socket>
    struct transfer<Socket, data_stream<Socket>> {
        using T = data_stream<Socket>;

        static auto read(Socket& sock) -> T {
            return T(sock);
        }
    };
}
