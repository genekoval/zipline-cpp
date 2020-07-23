#pragma once

namespace zipline {
    template <typename Protocol, typename EventT>
    class client {
        Protocol proto;

        template <typename ...Args>
        auto write(EventT event, Args&&... args) -> void {
            proto.write(event);
            ((
                proto.write(args)
            ), ...);
        }
    public:
        client(const Protocol::socket_type& sock) : proto(sock) {}

        template <typename ...Args>
        auto emit(EventT event, Args&&... args) -> void {
            write(event, args...);
            proto.wait_for_ack();
        }

        template <typename R, typename ...Args>
        auto send(EventT event, Args&&... args) -> R {
            write(event, args...);
            return proto.template response<R>();
        }
    };
}
