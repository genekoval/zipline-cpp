#pragma once

#include <zipline/protocol.h>

#include <array>
#include <timber/timber>

namespace zipline {
    template <typename Protocol, typename EventT, std::size_t N>
    class router {
        using handler = void (*)(Protocol&);

        const std::array<handler, N> endpoints;

        auto get(EventT event) const -> handler {
            try {
                return endpoints.at(event);
            }
            catch (const std::out_of_range&) {
                return nullptr;
            }
        }
    public:
        template <typename ...Events>
        router(Events&&... events) : endpoints { events... } {}

        auto route(const Protocol::socket_type& sock) const -> void {
            auto proto = Protocol(sock);
            auto event = proto.template read<EventT>();

            DEBUG() << "Event received: " << event;

            auto endpoint = get(event);
            if (!endpoint) {
                ERROR() << "Event (" << event << ") does not exist";
                proto.error("no such event");
                return;
            }

            try {
                endpoint(proto);
            }
            catch (const std::exception& ex) {
                proto.error(ex.what());
            }
        }
    };

    template <
        typename Protocol,
        typename EventT,
        typename ...Events,
        typename Router = router<Protocol, EventT, sizeof...(Events)>
    >
    auto make_router(
        Events&&... events
    ) -> Router {
        DEBUG()
            << "Generating event router with ("
            << sizeof...(Events)
            << ") events";
        return Router(events...);
    }
}
