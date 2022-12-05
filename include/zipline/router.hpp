#pragma once

#include "error.h"
#include "server_protocol.hpp"

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
    requires
        io::reader<Socket> && io::writer<Socket> &&
        std::unsigned_integral<EventT> && codable<EventT, Socket>
    class router {
        using protocol_type = server_protocol<Context, Socket, ErrorList>;
        using route_type = auto (*)(
            const std::tuple<Routes...>&,
            protocol_type&
        ) -> ext::task<>;

        Context ctx;
        const ErrorList errors;
        std::array<route_type, sizeof...(Routes)> routes;
        std::tuple<Routes...> route_source;

        template <std::size_t ...I>
        auto initialize(std::index_sequence<I...>) -> void {
            ((routes[I] = [](
                const std::tuple<Routes...>& source,
                protocol_type& proto
            ) -> ext::task<> {
                co_await proto.use(std::get<I>(source));
            }), ...);
        }

        auto route_one(protocol_type& proto) -> ext::task<bool> {
            auto event = EventT();
            auto route = static_cast<route_type>(nullptr);

            try {
                event = co_await proto.template read<EventT>();
            }
            catch (const eof&) {
                TIMBER_DEBUG("client closed connection");
                co_return false;
            }

            TIMBER_DEBUG("received event: {}", event);

            try {
                route = routes.at(event);
            }
            catch (const std::out_of_range&) {
                TIMBER_ERROR("unknown event received: {}", event);
                co_return false;
            }

            co_await route(route_source, proto);
            TIMBER_DEBUG("event handled: {}", event);

            co_return true;
        }
    public:
        router(Context&& ctx, Routes... r) :
            ctx(std::move(ctx)),
            route_source(r...)
        {
            initialize(std::index_sequence_for<Routes...>());

            TIMBER_TRACE(
                "Created router with ({:L}) routes",
                this->routes.size()
            );
        }

        auto route(Socket& sock) -> ext::task<> {
            auto proto = protocol_type(ctx, sock, errors);
            auto run = true;

            while (run) run = co_await route_one(proto);
        }

        auto route_one(Socket& sock) -> ext::task<> {
            auto proto = protocol_type(ctx, sock, errors);
            co_await route_one(proto);
        }
    };
}
