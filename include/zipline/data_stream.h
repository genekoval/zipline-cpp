#pragma once

#include <zipline/transfer.h>

#include <array>
#include <cstddef>
#include <functional>
#include <span>

namespace zipline {
    template <typename Socket>
    class data_stream {
        using size_type = std::size_t;

        static constexpr auto buffer_size = 8192;

        std::array<char, buffer_size> buffer;
        const Socket* sock;
    public:
        data_stream(const Socket& sock) : sock(&sock) {}

        template <typename Callable>
        auto read(Callable callback) -> void {
            const auto total = transfer<Socket, size_type>::read(*sock);
            auto received = size_type();

            do {
                const auto bytes = sock->recv(buffer.data(), buffer.size());
                received += bytes;

                callback(std::span(buffer.begin(), bytes));
            } while (received < total);
        }
    };

    template <typename Socket>
    struct transfer<Socket, data_stream<Socket>> {
        using T = data_stream<Socket>;

        static auto read(const Socket& sock) -> T {
            auto stream = T(sock);
            return stream;
        }
    };
}
