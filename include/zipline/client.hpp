#pragma once

#include "protocol.hpp"

#include <span>

namespace zipline {
    template <typename Socket, typename EventT, typename ErrorList>
    requires
        io::reader<Socket> &&
        io::writer<Socket> &&
        codable<EventT, Socket>
    class client {
        using protocol_type = protocol<Socket, ErrorList>;

        template <typename T>
        using response_type = response<T, Socket, ErrorList>;
    protected:
        protocol_type proto;
    public:
        client() = default;

        client(const ErrorList& errors, Socket& socket) :
            proto(socket, errors)
        {}

        template <typename T>
        auto read() const -> ext::task<T> {
            co_return co_await proto.template read<T>();
        }

        template <typename T>
        auto read(const response_type<T>& res) const -> ext::task<T> {
            co_return co_await res.read(*(proto.errors));
        }

        template <typename T = void>
        auto response() const -> ext::task<T> {
            co_return co_await proto.template response<T>();
        }

        template <typename R, typename ...Args>
        auto send(EventT event, const Args&... args) -> ext::task<R> {
            co_await start(event, args...);
            co_return co_await response<R>();
        }

        template <typename ...Args>
        auto start(EventT event, const Args&... args) const -> ext::task<> {
            co_await write(event, args...);
        }

        template <typename ...Args>
        auto write(const Args&... args) const -> ext::task<> {
            (co_await proto.write(args), ...);
        }
    };
}
