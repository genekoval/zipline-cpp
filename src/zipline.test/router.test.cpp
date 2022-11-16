#include "socket.test.h"

#include <zipline/zipline>

#include <cstring>
#include <fmt/format.h>

using namespace std::literals;

using socket = zipline::memory_buffer;

namespace {
    struct context {
        std::string& storage;

        explicit context(std::string& storage) : storage(storage) {}

        auto greet(std::string name) -> ext::task<std::string> {
            co_return fmt::format("Hello, {}!", name);
        }

        auto save(std::string value) -> ext::task<> {
            storage = value;
            co_return;
        }

        auto set_default() -> ext::task<> {
            storage = "default";
            co_return;
        }
    };

    using event_type = std::uint16_t;

    enum class events : event_type {
        greet,
        save,
        set_default
    };

    using error_list = zipline::error_list<socket>;

    using router = zipline::router<
        socket,
        event_type,
        error_list,
        context,
        decltype(&context::greet),
        decltype(&context::save),
        decltype(&context::set_default)
    >;

    auto make_router(std::string& storage) -> router {
        return router(
            context(storage),
            &context::greet,
            &context::save,
            &context::set_default
        );
    }
}

class RouterTest : public SocketTestBase {
protected:
    const error_list errors;

    std::string storage;

    zipline::client<zipline::memory_buffer, events, error_list> client =
        {errors, socket};

    router routes = make_router(storage);

    auto route() -> ext::task<> {
        return routes.route_one(socket);
    }
};

TEST_F(RouterTest, MemberRoute) {
    [this]() -> ext::detached_task {
        constexpr auto world = "world"sv;
        co_await client.start(events::greet, world);
        co_await route();
        const auto response = co_await client.response<std::string>();
        EXPECT_EQ("Hello, world!", response);

        constexpr auto value = "my value"sv;
        co_await client.start(events::save, value);
        co_await route();
        co_await client.response<>();
        EXPECT_EQ(value, storage);

        co_await client.start(events::set_default);
        co_await route();
        co_await client.response<>();
        EXPECT_EQ("default"sv, storage);
    }();
}
