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
        auto read(Callable pipe) -> ext::task<> {
            auto remaining = co_await size();

            TIMBER_TRACE("reading data stream: {:L} bytes", remaining);

            co_await pipe(co_await peek());
            remaining -= chunk.size();

            while (remaining > 0) {
                chunk = co_await socket->read(remaining);
                remaining -= chunk.size();

                co_await pipe(chunk);

                TIMBER_TRACE("stream has {:L} bytes remaining", remaining);
            }
        }

        auto peek() -> ext::task<std::span<const std::byte>> {
            if (!chunk.data()) chunk = co_await socket->read(co_await size());
            co_return chunk;
        }

        auto size() -> ext::task<std::size_t> {
            if (!stream_size) {
                stream_size = co_await decode_size(*socket);
            }

            co_return *stream_size;
        }
    };

    template <typename Socket>
    struct coder<Socket, data_stream<Socket>> {
        static auto decode(Socket& socket) -> ext::task<data_stream<Socket>> {
            co_return data_stream<Socket>(socket);
        }
    };
}
