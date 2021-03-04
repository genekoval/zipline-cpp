#pragma once

namespace zipline {
    template <typename Protocol, typename EventT>
    class client {
        Protocol::socket_type socket;
    protected:
        Protocol proto;
    public:
        client(Protocol::socket_type&& socket) :
            socket(std::move(socket)),
            proto(this->socket)
        {}

        template <typename ...Args>
        auto emit(EventT event, Args&&... args) -> void {
            write(event, args...);
            proto.wait_for_ack();
        }

        template <typename R>
        auto response() -> R {
            return proto.template response<R>();
        }

        template <typename R, typename ...Args>
        auto send(EventT event, Args&&... args) -> R {
            write(event, args...);
            return proto.template response<R>();
        }

        template <typename ...Args>
        auto write(Args&&... args) -> void {
            ((proto.write(args)), ...);
        }

        auto write_bytes(std::span<const std::byte> bytes) -> void {
            proto.write_bytes(bytes);
        }
    };
}
