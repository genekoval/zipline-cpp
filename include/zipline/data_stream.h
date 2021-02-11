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
        const Socket* sock;
    public:
        data_stream(const Socket& sock) : sock(&sock) {}

        template <typename Callable>
        auto read(Callable pipe) -> void {
            const auto total = transfer<Socket, size_type>::read(*sock);
            DEBUG() << *sock << " reading data stream: " << total << " bytes";

            auto remaining = total;

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
    };

    template <typename Socket>
    struct transfer<Socket, data_stream<Socket>> {
        using T = data_stream<Socket>;

        static auto read(const Socket& sock) -> T {
            return T(sock);
        }
    };
}
