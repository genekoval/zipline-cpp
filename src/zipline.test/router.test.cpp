#include "socket.test.h"

#include <zipline/zipline>

#include <cstring>
#include <gtest/gtest.h>

using socket = zipline::test::socket;

namespace {
    struct context {
        std::string storage;

        auto greet(std::string name) -> std::string {
            return "Hello, " + name + "!";
        }

        auto save(std::string value) -> void {
            storage = value;
        }

        auto set_default() -> void {
            storage = "default";
        }
    };

    using event_type = std::uint16_t;

    template <typename ...Errors>
    using error_list = zipline::error_list<socket, Errors...>;

    using errors = error_list<>;

    template <typename ...Routes>
    using router = zipline::router<
        socket,
        event_type,
        errors,
        context,
        Routes...
    >;
}

class RouterTest : public SocketTestBase {};

TEST_F(RouterTest, MemberRoute) {
    const auto e = errors();
    auto ctx = context();
    const auto rt = router(
        ctx,
        &context::greet,
        &context::save,
        &context::set_default
    );

    auto proto = zipline::protocol<socket, errors>(sock, e);

    proto.write<event_type>(0);
    proto.write<std::string>("world");

    rt.route(sock);

    ASSERT_EQ("Hello, world!", proto.response<std::string>());

    proto.write<event_type>(1);
    proto.write<std::string>("my value");

    rt.route(sock);
    proto.response<void>();

    ASSERT_EQ("my value", ctx.storage);

    proto.write<event_type>(2);

    rt.route(sock);
    proto.response<void>();

    ASSERT_EQ("default", ctx.storage);
}
