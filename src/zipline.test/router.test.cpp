#include <zipline/zipline>

#include <cstring>
#include <gtest/gtest.h>

class test_socket {
    std::array<std::byte, 1024> buffer;
    std::size_t head = 0;
    std::size_t tail = 0;
public:
    auto flush() -> void {}

    auto read(void* dest, std::size_t len) -> std::size_t {
        std::memcpy(dest, &buffer[head], len);
        head += len;

        if (head == tail) {
            head = 0;
            tail = 0;
        }

        return len;
    }

    auto write(const void* src, std::size_t len) -> std::size_t {
        std::memcpy(&buffer[tail], src, len);
        tail += len;

        return len;
    }
};

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
using error_list = zipline::error_list<test_socket, Errors...>;

using errors = error_list<>;

template <typename ...Routes>
using router = zipline::router<
    test_socket,
    event_type,
    errors,
    context,
    Routes...
>;

TEST(RouterTest, MemberRoute) {
    const auto e = errors();
    auto ctx = context();
    const auto rt = router(
        ctx,
        &context::greet,
        &context::save,
        &context::set_default
    );

    auto sock = test_socket();
    auto proto = zipline::protocol<test_socket, errors>(sock, e);

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
