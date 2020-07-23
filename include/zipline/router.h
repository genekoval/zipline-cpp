#pragma once

#include <zipline/protocol.h>

#include <array>
#include <timber/timber>

namespace zipline {
    template <typename Protocol, typename EventT, std::size_t N>
    class router {
        using handler = void (*)(Protocol&);

        const std::array<handler, N> endpoints;
    public:
        template <typename ...Events>
        router(Events&&... events) : endpoints { events... } {}

        auto route(const Protocol::socket_type& sock) const -> void {
            auto proto = Protocol(sock);
            auto event = proto.template read<EventT>();

            DEBUG() << "Event received: " << event;

            handler endpoint = nullptr;

            try {
                endpoint = endpoints.at(event);
            }
            catch (const std::out_of_range& ex) {
                ERROR() << "Event (" << event << ") does not exist";

                proto.write(false);
                proto.write(std::string("no such event"));

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
        typename ...Events
    >
    auto make_router(
        Events&&... events
    ) -> router<Protocol, EventT, sizeof...(Events)> {
        DEBUG()
            << "Generating event router with ("
            << sizeof...(Events)
            << ") events";
        return router<Protocol, EventT, sizeof...(Events)>(events...);
    }
}
