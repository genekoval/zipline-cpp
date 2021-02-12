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

        static constexpr auto buffer_size = 8192UL;

        std::array<char, buffer_size> buffer;
        bool ready = false;
        const Socket* sock;
        size_type stream_size;
    public:
        data_stream(const Socket& sock) : sock(&sock) {}

        template <typename Callable>
        auto read(Callable pipe) -> void {
            prepare();

            DEBUG()
                << *sock << " reading data stream: " << stream_size << " bytes";

            auto remaining = stream_size;

            while (remaining > 0) {
                const auto& size = std::min(remaining, buffer_size);
                const auto bytes = sock->recv(buffer.data(), size);

                remaining -= bytes;

                if (bytes == 0) {
                    ERROR()
                        << "Data stream received EOF with "
                        << remaining << " bytes remaining";
                    throw std::runtime_error("received EOF");
                }

                pipe(std::span(buffer.begin(), bytes));
            }
        }

        auto prepare() -> void {
            if (ready) return;

            stream_size = transfer<Socket, size_type>::read(*sock);
            ready = true;
        }

        auto size() -> size_type {
            if (!ready) throw std::runtime_error(
                "cannot get size: data stream not ready"
            );
            return stream_size;
        }
    };

    template <typename Socket>
    struct transfer<Socket, data_stream<Socket>> {
        using T = data_stream<Socket>;

        static auto read(const Socket& sock) -> T {
            return T(sock);
        }
    };
}
