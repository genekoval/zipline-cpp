#pragma once

namespace zipline {
    template <typename Protocol, typename EventT>
    class client {
        Protocol::socket_type socket;
        Protocol proto;

        template <typename ...Args>
        auto write(EventT event, Args&&... args) -> void {
            proto.write(event);
            ((
                proto.write(args)
            ), ...);
        }
    public:
        client(Protocol::socket_type&& socket) :
            socket(std::move(socket)),
            proto(this->socket)
        {}

        template <typename ...Args>
        auto emit(EventT event, Args&&... args) -> void {
            write(event, args...);
            proto.end();
            proto.wait_for_ack();
        }

        template <typename R, typename ...Args>
        auto send(EventT event, Args&&... args) -> R {
            write(event, args...);
            proto.end();
            return proto.template response<R>();
        }
    };
}
