#include <zipline/server_protocol.h>
#include <zipline/zipline>

#include <cstring>
#include <gtest/gtest.h>

using namespace std::literals;

struct TestContext {
    std::string result;
};

class TestSocket {
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

using protocol = zipline::server_protocol<TestContext, TestSocket>;

constexpr auto test_string = "test";
constexpr auto test_int = 500;

auto multiply_by_two(
    TestContext* ctx,
    int a,
    int b,
    int c
) -> std::vector<int> {
    return { a * 2, b * 2, c * 2 };
}

auto concat(
    TestContext* ctx,
    std::string str,
    int i
) -> void {
    ctx->result.append(str);
    ctx->result.append(std::to_string(i));
}

TEST(ServerProtocol, UseWithReturn) {
    const auto numbers = std::vector<int> { 1, 2, 3 };
    const auto expected = std::vector<int> { 2, 4, 6 };

    auto context = TestContext();
    auto socket = TestSocket();

    auto proto = protocol(context, socket);

    for (const auto n : numbers) proto.write(n);

    proto.use(multiply_by_two);
    const auto result = proto.response<std::vector<int>>();

    ASSERT_EQ(expected, result);
}

TEST(ServerProtocol, UseWithVoid) {
    const auto one = "foo"s;
    const auto two = 500;
    constexpr auto expected = "foo500";

    auto context = TestContext();
    auto socket = TestSocket();

    auto proto = protocol(context, socket);

    proto.write(one);
    proto.write(two);

    proto.use(concat);
    proto.wait_for_ack();

    ASSERT_EQ(expected, context.result);
}
