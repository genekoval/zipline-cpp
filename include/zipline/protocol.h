#pragma once

#include <zipline/transfer.h>

#include <string>
#include <string_view>
#include <timber/timber>
#include <type_traits>

namespace zipline {
    struct default_error_handler {
        template <typename Socket>
        static auto throw_error(const Socket& sock) -> void {
            throw std::runtime_error(transfer<Socket, std::string>::read(sock));
        }
    };

    template <typename Socket, typename ErrorHandler = default_error_handler>
    class protocol {
        auto unhandled_error() const -> std::runtime_error {
            return std::runtime_error("unhandled zipline error");
        }
    protected:
        const Socket* sock;
    public:
        using socket_type = Socket;

        protocol(const Socket& sock) : sock(&sock) {}

        auto end() const -> void {
            sock->end();
        }

        template <typename T, typename U = std::remove_reference_t<T>>
        auto read() const -> U {
            return transfer<Socket, U>::read(*sock);
        }

        auto reply() -> void {
            write_success();
        }

        template <typename T>
        auto reply(T&& t) -> void {
            write_success();
            write(t);
        }

        template <typename T>
        auto response() const -> T {
            if (read<bool>()) return read<T>();
            ErrorHandler::template throw_error<Socket>(*sock);
            throw unhandled_error();
        }

        auto wait_for_ack() -> void {
            if (read<bool>()) return;
            ErrorHandler::template throw_error<Socket>(*sock);
            throw unhandled_error();
        }

        template <typename T>
        auto write(const T& t) const -> void {
            transfer<Socket, T>::write(*sock, t);
        }

        auto write_error(std::string_view message) const -> void {
            write_failure();
            write(message);
        }

        auto write_failure() const -> void {
            write(false);
            DEBUG() << "write failure: " << false;
        }

        auto write_success() const -> void {
            write(true);
            DEBUG() << "write success: " << true;
        }
    };
}
