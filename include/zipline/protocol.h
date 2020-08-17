#pragma once

#include <zipline/transfer.h>

#include <string>
#include <string_view>
#include <type_traits>

namespace zipline {
    template <typename Socket>
    class protocol {
    protected:
        const Socket* sock;
    public:
        using socket_type = Socket;

        protocol(const Socket& sock) : sock(&sock) {}

        template <typename T, typename U = std::remove_reference_t<T>>
        auto read() const -> U {
            return transfer<Socket, U>::read(*sock);
        }

        auto read_error() const -> std::runtime_error {
            return std::runtime_error(read<std::string>());
        }
        template <typename R, typename ...Args>
        auto reply(R (*callable)(Args...)) -> void {
            const auto result = callable((read<Args>(), ...));

            write(true);
            write(result);
        }

        template <typename ...Args>
        auto reply(void (*callable)(Args...)) -> void {
            callable((read<Args>(), ...));
            write(true);
        }

        template <typename T>
        auto response() const -> T {
            if (read<bool>()) return read<T>();
            throw read_error();
        }

        auto wait_for_ack() -> void {
            if (read<bool>()) return;
            throw read_error();
        }

        template <typename T>
        auto write(const T& t) const -> void {
            transfer<Socket, T>::write(*sock, t);
        }

        auto write_error(std::string_view message) const -> void {
            write(false);
            write(message);
        }
    };
}
