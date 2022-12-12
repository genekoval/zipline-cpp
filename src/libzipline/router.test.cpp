#include "socket.test.hpp"

#include <cstring>
#include <fmt/format.h>

using namespace std::literals;

using zipline::test::buffer_type;

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

    enum class event : std::uint16_t {
        greet,
        save,
        set_default
    };

    namespace detail {
        template <typename... Routes>
        auto make_router(context&& ctx, Routes&&... routes) {
            return zipline::router<
                buffer_type,
                std::underlying_type_t<event>,
                context,
                Routes...
            >(
                std::forward<context>(ctx),
                zipline::error_list<>::codes(),
                std::forward<Routes>(routes)...
            );
        }
    }

    auto make_router(std::string& storage) {
        return detail::make_router(
            context(storage),
            &context::greet,
            &context::save,
            &context::set_default
        );
    }

    using router_type = std::invoke_result_t<
        decltype(make_router),
        std::string&
    >;

    using client_type = zipline::client<buffer_type&, event>;
}

class RouterTest : public SocketTestBase {
protected:
    std::string storage;

    router_type router = make_router(storage);

    client_type client = client_type(
        zipline::error_list<>::thrower(),
        buffer
    );

    auto route() -> ext::task<> {
        return router.route_one(buffer);
    }
};

TEST_F(RouterTest, MemberRoute) {
    [this]() -> ext::detached_task {
        constexpr auto world = "world"sv;
        co_await client.start(event::greet, world);
        co_await route();
        const auto response = co_await client.read_response<std::string>();
        EXPECT_EQ("Hello, world!", response);

        constexpr auto value = "my value"sv;
        co_await client.start(event::save, value);
        co_await route();
        co_await client.read_response();
        EXPECT_EQ(value, storage);

        co_await client.start(event::set_default);
        co_await route();
        co_await client.read_response();
        EXPECT_EQ("default"sv, storage);
    }();
}
