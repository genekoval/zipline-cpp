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

        auto error(std::string_view message) const -> void {
            write(false);
            write(message);
        }

        template <typename T>
        auto read() const -> T {
            return transfer<Socket, T>::read(*sock);
        }

        auto reply() const -> void { write(true); }

        template <typename T>
        auto reply(const T& t) const -> void {
            write(true);
            write(t);
        }

        template <typename T>
        auto response() const -> T {
            if (read<bool>()) return read<T>();
            else throw std::runtime_error(read<std::string>());
        }

        auto wait_for_ack() -> void {
            if (read<bool>()) return;
            else throw std::runtime_error(read<std::string>());
        }

        template <typename T>
        auto write(const T& t) const -> void {
            transfer<Socket, T>::write(*sock, t);
        }
    };
}
