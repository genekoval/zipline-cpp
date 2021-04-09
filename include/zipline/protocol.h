#pragma once

#include <zipline/transfer.h>

#include <string>
#include <string_view>
#include <timber/timber>
#include <type_traits>

namespace zipline {
    template <typename Socket>
    struct default_error_handler {
        static auto throw_error(Socket& sock) -> void {
            throw std::runtime_error(transfer<Socket, std::string>::read(sock));
        }
    };

    template <
        typename Socket,
        typename ErrorHandler = default_error_handler<Socket>
    >
    class protocol {
        auto unhandled_error() const -> std::runtime_error {
            return std::runtime_error("unhandled zipline error");
        }

        auto wait() const -> bool {
            sock->flush();
            return read<bool>();
        }
    protected:
        Socket* sock;
    public:
        using socket_type = Socket;

        protocol(Socket& sock) : sock(&sock) {}

        template <typename T, typename U = std::remove_reference_t<T>>
        auto read() const -> U {
            return transfer<Socket, U>::read(*sock);
        }

        auto reply() const -> void {
            write_success();
        }

        template <typename T>
        auto reply(T&& t) const -> void {
            write_success();
            write(t);
        }

        template <typename T>
        auto response() const -> T {
            if (wait()) return read<T>();
            ErrorHandler::throw_error(*sock);
            throw unhandled_error();
        }

        auto wait_for_ack() -> void {
            if (wait()) return;
            ErrorHandler::throw_error(*sock);
            throw unhandled_error();
        }

        template <typename T>
        auto write(const T& t) const -> void {
            transfer<Socket, T>::write(*sock, t);
        }

        auto write_bytes(std::span<const std::byte> bytes) -> void {
            sock->write(bytes.data(), bytes.size());
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
