#pragma once

#define ZIPLINE_ROUTE(ns, protocol, route) \
    auto route(protocol& proto) -> void {\
        proto.use(ns::route);\
    }

#define ZIPLINE_ROUTER(protocol, event, ...) \
    static inline auto router() {\
        return zipline::make_router<protocol, event>(__VA_ARGS__);\
    }
