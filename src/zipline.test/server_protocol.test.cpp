#include <zipline/zipline>

#include <cstring>
#include <gtest/gtest.h>

using namespace std::literals;

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

template <typename Derived>
using zipline_error = zipline::zipline_error<TestSocket, Derived>;

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
    struct transfer<TestSocket, custom_error> {
        static auto read(TestSocket& sock) -> custom_error {
            const auto n = transfer<TestSocket, int>::read(sock);
            const auto message = transfer<TestSocket, std::string>::read(sock);

            return custom_error(n, message);
        }

        static auto write(TestSocket& sock, const custom_error& error) -> void {
            transfer<TestSocket, int>::write(sock, error.number());
            transfer<TestSocket, std::string>::write(sock, error.what());
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

using error_list = zipline::error_list<TestSocket, custom_error>;

using protocol = zipline::server_protocol<
    TestContext,
    TestSocket,
    error_list
>;

TEST(ServerProtocol, UseWithReturn) {
    const auto numbers = std::vector<int> { 1, 2, 3 };
    const auto expected = std::vector<int> { 2, 4, 6 };

    const auto errors = error_list();

    auto context = TestContext();
    auto socket = TestSocket();

    auto proto = protocol(context, socket, errors);

    for (const auto n : numbers) proto.write(n);

    proto.use(&TestContext::multiply_by_two);
    const auto result = proto.response<std::vector<int>>();

    ASSERT_EQ(expected, result);
}

TEST(ServerProtocol, UseWithVoid) {
    const auto one = "foo"s;
    const auto two = 500;
    constexpr auto expected = "foo500";

    const auto errors = error_list();

    auto context = TestContext();
    auto socket = TestSocket();

    auto proto = protocol(context, socket, errors);

    proto.write(one);
    proto.write(two);

    proto.use(&TestContext::concat);
    proto.response<void>();

    ASSERT_EQ(expected, context.result);
}

TEST(ServerProtocol, UseWithThrow) {
    auto context = TestContext();
    auto socket = TestSocket();

    const auto errors = error_list();

    auto proto = protocol(context, socket, errors);

    try {
        proto.use(&TestContext::throw_error);
        proto.response<void>();

        FAIL() << "An error should have been thrown.";
    }
    catch (const std::runtime_error& ex) {
        ASSERT_EQ(error_string, ex.what());
    }
}

TEST(ServerProtocol, UseWithCustomThrow) {
    auto context = TestContext();
    auto socket = TestSocket();

    const auto errors = error_list();

    auto proto = protocol(context, socket, errors);

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
