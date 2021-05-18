#pragma once

#include <zipline/server_protocol.h>

#include <array>
#include <timber/timber>

namespace zipline {
    template <
        typename Socket,
        typename EventT,
        typename ErrorList,
        typename Context,
        typename ...Routes
    >
    class router {
        using protocol_type = server_protocol<Context, Socket, ErrorList>;
        using route_type = void (*)(
            const std::tuple<Routes...>&,
            protocol_type&
        );

        Context* ctx;
        const ErrorList errors;
        std::array<route_type, sizeof...(Routes)> routes;
        std::tuple<Routes...> route_source;

        template <std::size_t ...I>
        auto initialize(std::index_sequence<I...>) -> void {
            ((routes[I] = [](
                const std::tuple<Routes...>& source,
                protocol_type& proto
            ) -> void {
                proto.use(std::get<I>(source));
            }), ...);
        }
    public:
        router(Context& ctx, Routes... r) :
            ctx(&ctx),
            route_source(r...)
        {
            initialize(std::index_sequence_for<Routes...>());
            DEBUG()
                << "Created router with ("
                << this->routes.size()
                << ") routes";
        }

        auto route(Socket& sock) const -> void {
            auto proto = protocol_type(*ctx, sock, errors);
            const auto event = proto.template read<EventT>();

            DEBUG() << "event received: " << event;

            try {
                routes.at(event)(route_source, proto);
                DEBUG() << "event handled: " << event;
            }
            catch (const std::out_of_range&) {
                ERROR() << "unknown event received: " << event;
            }
        }
    };
}
