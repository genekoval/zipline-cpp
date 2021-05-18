#pragma once

#include <zipline/protocol.h>

#include <span>

namespace zipline {
    template <typename Socket, typename EventT, typename ErrorList>
    class client {
        using protocol_type = protocol<Socket, ErrorList>;

        template <typename T>
        using response_type = response<T, Socket, ErrorList>;

        Socket socket;
    protected:
        protocol_type proto;
    public:
        client(const ErrorList& errors, Socket&& socket) :
            socket(std::move(socket)),
            proto(this->socket, errors)
        {}

        template <typename T>
        auto read() const -> std::remove_reference_t<T> {
            return proto.template read<T>();
        }

        template <typename T>
        auto read(const response_type<T>& res) const -> T {
            return res.read(*(proto.errors));
        }

        template <typename T>
        auto response() const -> T {
            return proto.template response<T>();
        }

        template <typename R, typename ...Args>
        auto send(EventT event, const Args&... args) -> R {
            start(event, args...);
            return response<R>();
        }

        template <typename ...Args>
        auto start(EventT event, const Args&... args) const -> void {
            write(event, args...);
        }

        template <typename ...Args>
        auto write(const Args&... args) const -> void {
            ((proto.write(args)), ...);
        }

        auto write_bytes(std::span<const std::byte> bytes) -> void {
            proto.write_bytes(bytes);
        }
    };
}
