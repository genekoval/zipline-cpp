#pragma once

#include <zipline/transfer.h>

#include <string>
#include <string_view>

namespace zipline {
    template <typename Socket>
    class protocol {
        const Socket* sock;
    public:
        using socket_type = Socket;

        protocol(const Socket& sock) : sock(&sock) {}

        template <typename T>
        auto read() const -> T {
            return transfer<Socket, T>::read(*sock);
        }

        template <typename T>
        auto write(const T& t) const -> void {
            transfer<Socket, T>::write(*sock, t);
        }
    };
}
