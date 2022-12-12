#pragma once

#include "error.h"
#include "server_protocol.hpp"

#include <array>
#include <timber/timber>

namespace zipline {
    template <
        typename Socket,
        typename EventT,
        typename Context,
        typename ...Routes
    >
    requires
        io::reader<Socket> && io::writer<Socket> &&
        std::unsigned_integral<EventT> && decodable<EventT, Socket>
    class router {
        using protocol = server_protocol<Context, Socket>;
        using route_storage = std::tuple<Routes...>;
        using route_type = auto (*)(
            const std::tuple<Routes...>&,
            protocol&
        ) -> ext::task<>;

        Context ctx;
        const error_codes& errors;
        std::array<route_type, sizeof...(Routes)> routes;
        route_storage storage;

        template <std::size_t ...I>
        auto initialize(std::index_sequence<I...>) -> void {
            ((routes[I] = [](
                const route_storage& storage,
                protocol& proto
            ) -> ext::task<> {
                co_await proto.use(std::get<I>(storage));
            }), ...);
        }

        auto route_one(protocol& proto) -> ext::task<bool> {
            auto event = EventT();
            route_type route = nullptr;

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

            co_await route(storage, proto);
            TIMBER_DEBUG("event handled: {}", event);

            co_return true;
        }
    public:
        router(Context&& ctx, const error_codes& errors, Routes&&... r) :
            ctx(std::forward<Context>(ctx)),
            errors(errors),
            storage(std::forward<Routes>(r)...)
        {
            initialize(std::index_sequence_for<Routes...>());

            TIMBER_TRACE(
                "Created router with ({:L}) routes",
                this->routes.size()
            );
        }

        auto route(Socket& sock) -> ext::task<> {
            auto proto = protocol(ctx, errors, sock);
            auto run = true;

            while (run) run = co_await route_one(proto);
        }

        auto route_one(Socket& sock) -> ext::task<> {
            auto proto = protocol(ctx, errors, sock);
            co_await route_one(proto);
        }
    };
}
