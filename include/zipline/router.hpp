#pragma once

#include "error.h"
#include "server_protocol.hpp"

#include <array>
#include <timber/timber>

namespace zipline {
    template <typename Context>
    concept router_context = requires(Context ctx, std::chrono::seconds sec) {
        { ctx.set_timer(sec) } -> std::same_as<ext::task<>>;
    };

    template <
        typename Socket,
        typename EventT,
        typename Context,
        typename... Routes>
    requires io::reader<Socket> && io::writer<Socket> &&
             std::unsigned_integral<EventT> && decodable<EventT, Socket> &&
             router_context<Context>
    class router {
        using protocol = server_protocol<Context, Socket>;
        using route_storage = std::tuple<Routes...>;
        using route_type = auto (*)(const std::tuple<Routes...>&, protocol&)
            -> ext::task<>;
        using seconds = std::chrono::seconds;

        Context ctx;
        const error_codes& errors;
        std::array<route_type, sizeof...(Routes)> routes;
        route_storage storage;

        template <std::size_t... I>
        auto initialize(std::index_sequence<I...>) -> void {
            ((routes[I] = [](const route_storage& storage, protocol& proto)
                  -> ext::task<> { co_await proto.use(std::get<I>(storage)); }),
             ...);
        }

        auto route_one(protocol& proto, seconds timeout) -> ext::task<bool> {
            auto fill_buffer = proto.fill_buffer();

            if (timeout > seconds::zero()) {
                auto result = co_await ext::race(
                    std::move(fill_buffer),
                    ctx.set_timer(timeout)
                );

                if (result.index() == 1) {
                    co_await std::get<1>(std::move(result));
                    TIMBER_DEBUG("Client connection idle timeout reached");
                    co_return false;
                }

                fill_buffer = std::get<0>(std::move(result));
            }

            if (!co_await std::exchange(fill_buffer, {})) {
                TIMBER_DEBUG("Client disconnected");
                co_return false;
            }

            const auto event = co_await proto.template read<EventT>();
            TIMBER_DEBUG("Received event: {}", event);

            route_type route = nullptr;
            try {
                route = routes.at(event);
            }
            catch (const std::out_of_range&) {
                TIMBER_ERROR("Unknown event received: {}", event);
                co_return false;
            }

            co_await route(storage, proto);

            TIMBER_DEBUG("Event handled: {}", event);
            co_return true;
        }
    public:
        router(Context&& ctx, const error_codes& errors, Routes&&... r) :
            ctx(std::forward<Context>(ctx)),
            errors(errors),
            storage(std::forward<Routes>(r)...) {
            initialize(std::index_sequence_for<Routes...>());

            TIMBER_TRACE(
                "Created router with ({:L}) routes",
                this->routes.size()
            );
        }

        auto route(Socket& sock, seconds timeout = seconds::zero())
            -> ext::task<> {
            auto proto = protocol(ctx, errors, sock);
            auto run = true;

            while (run) run = co_await route_one(proto, timeout);
        }

        auto route_one(Socket& sock, seconds timeout = seconds::zero())
            -> ext::task<> {
            auto proto = protocol(ctx, errors, sock);
            co_await route_one(proto, timeout);
        }
    };
}
