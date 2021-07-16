#include "socket.test.h"

#include <zipline/zipline>

#include <cstring>
#include <gtest/gtest.h>

using namespace std::literals;

using socket = zipline::test::socket;

template <typename Derived>
using zipline_error = zipline::zipline_error<socket, Derived>;

class custom_error : public zipline_error<custom_error> {
    int n;
public:
    custom_error(int n, std::string message) :
        zipline_error(message),
        n(n)
    {}

    auto number() const -> int {
        return n;
    }
};

namespace zipline {
    template <>
    struct transfer<socket, custom_error> {
        static auto read(socket& sock) -> custom_error {
            const auto n = transfer<socket, int>::read(sock);
            const auto message = transfer<socket, std::string>::read(sock);

            return custom_error(n, message);
        }

        static auto write(socket& sock, const custom_error& error) -> void {
            transfer<socket, int>::write(sock, error.number());
            transfer<socket, std::string>::write(sock, error.what());
        }
    };
}

const auto error_string = "This is a test error."s;
constexpr auto number = 500;

struct TestContext {
    std::string result;

    auto multiply_by_two(
        int a,
        int b,
        int c
    ) -> std::vector<int> {
        return { a * 2, b * 2, c * 2 };
    }

    auto concat(
        std::string str,
        int i
    ) -> void {
        result.append(str);
        result.append(std::to_string(i));
    }

    auto throw_error() -> void {
        throw std::runtime_error(error_string);
    }

    auto throw_custom_error() -> void {
        throw custom_error(number, error_string);
    }
};

using error_list = zipline::error_list<socket, custom_error>;

using protocol = zipline::server_protocol<
    TestContext,
    socket,
    error_list
>;

class ServerProtocolTest : public SocketTestBase {};

TEST_F(ServerProtocolTest, UseWithReturn) {
    const auto numbers = std::vector<int> { 1, 2, 3 };
    const auto expected = std::vector<int> { 2, 4, 6 };

    const auto errors = error_list();

    auto context = TestContext();

    auto proto = protocol(context, sock, errors);

    for (const auto n : numbers) proto.write(n);

    proto.use(&TestContext::multiply_by_two);
    const auto result = proto.response<std::vector<int>>();

    ASSERT_EQ(expected, result);
}

TEST_F(ServerProtocolTest, UseWithVoid) {
    const auto one = "foo"s;
    const auto two = 500;
    constexpr auto expected = "foo500";

    const auto errors = error_list();

    auto context = TestContext();

    auto proto = protocol(context, sock, errors);

    proto.write(one);
    proto.write(two);

    proto.use(&TestContext::concat);
    proto.response<void>();

    ASSERT_EQ(expected, context.result);
}

TEST_F(ServerProtocolTest, UseWithThrow) {
    auto context = TestContext();

    const auto errors = error_list();

    auto proto = protocol(context, sock, errors);

    try {
        proto.use(&TestContext::throw_error);
        proto.response<void>();

        FAIL() << "An error should have been thrown.";
    }
    catch (const std::runtime_error& ex) {
        ASSERT_EQ(error_string, ex.what());
    }
}

TEST_F(ServerProtocolTest, UseWithCustomThrow) {
    auto context = TestContext();

    const auto errors = error_list();

    auto proto = protocol(context, sock, errors);

    try {
        proto.use(&TestContext::throw_custom_error);
        proto.response<void>();

        FAIL() << "An error should have been thrown.";
    }
    catch (const custom_error& ex) {
        ASSERT_EQ(number, ex.number());
        ASSERT_EQ(error_string, ex.what());
    }
}
