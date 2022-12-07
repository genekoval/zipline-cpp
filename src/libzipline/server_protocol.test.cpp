#include "socket.test.hpp"

#include <cstring>

using zipline::test::buffer_type;

namespace {
    using namespace std::literals;

    template <typename Derived>
    using zipline_error = zipline::zipline_error<buffer_type, Derived>;

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

    const auto error_string = "This is a test error."s;
    constexpr auto number = 500;

    struct TestContext {
        std::string result;

        auto multiply_by_two(
            std::int32_t a,
            std::int32_t b,
            std::int32_t c
        ) -> ext::task<std::vector<std::int32_t>> {
            co_return std::vector { a * 2, b * 2, c * 2 };
        }

        auto concat(
            std::string str,
            std::int32_t i
        ) -> ext::task<> {
            result.append(str);
            result.append(std::to_string(i));

            co_return;
        }

        auto throw_error() -> ext::task<> {
            throw std::runtime_error(error_string);
        }

        auto throw_custom_error() -> ext::task<> {
            throw custom_error(number, error_string);
        }
    };

    using error_list = zipline::error_list<buffer_type, custom_error>;

    using protocol = zipline::server_protocol<
        TestContext,
        buffer_type,
        error_list
    >;
}

namespace zipline {
    template <>
    struct decoder<custom_error, buffer_type> {
        static auto decode(buffer_type& buffer) -> ext::task<custom_error> {
            const auto n = co_await decoder<int, buffer_type>::decode(buffer);
            const auto message =
                co_await decoder<std::string, buffer_type>::decode(buffer);

            co_return custom_error(n, message);
        }
    };

    template <>
    struct encoder<custom_error, buffer_type> {
        static auto encode(
            const custom_error& error,
            buffer_type& buffer
        ) -> ext::task<> {
            co_await encoder<int, buffer_type>::encode(error.number(), buffer);
            co_await encoder<std::string, buffer_type>::encode(
                error.what(),
                buffer
            );
        }
    };
}

class ServerProtocolTest : public SocketTestBase {
protected:
    const error_list errors;
    TestContext context;
    protocol proto = protocol(context, buffer, errors);
};

TEST_F(ServerProtocolTest, UseWithReturn) {
    [this]() -> ext::detached_task {
        const auto numbers = std::vector<std::int32_t> { 1, 2, 3 };
        const auto expected = std::vector<std::int32_t> { 2, 4, 6 };

        for (const auto n : numbers) co_await proto.write(n);

        co_await proto.use(&TestContext::multiply_by_two);
        const auto result = co_await proto.response<std::vector<int>>();

        EXPECT_EQ(expected, result);
    }();
}

TEST_F(ServerProtocolTest, UseWithVoid) {
    [this]() -> ext::detached_task {
        constexpr auto expected = "foo500";

        const auto one = "foo"s;
        const std::int32_t two = 500;

        co_await proto.write(one);
        co_await proto.write(two);

        co_await proto.use(&TestContext::concat);
        co_await proto.response<void>();

        EXPECT_EQ(expected, context.result);
    }();
}

TEST_F(ServerProtocolTest, UseWithThrow) {
    auto fail = false;

    [&]() -> ext::detached_task {
        try {
            co_await proto.use(&TestContext::throw_error);
            co_await proto.response<void>();
            fail = true;
            co_return;
        }
        catch (const std::runtime_error& ex) {
            EXPECT_EQ(error_string, ex.what());
        }
    }();

    if (fail) FAIL() << "An error should have been thrown.";
}

TEST_F(ServerProtocolTest, UseWithCustomThrow) {
    auto fail = false;
    [&]() -> ext::detached_task {
        try {
            co_await proto.use(&TestContext::throw_custom_error);
            co_await proto.response<void>();
        }
        catch (const custom_error& ex) {
            EXPECT_EQ(number, ex.number());
            EXPECT_EQ(error_string, ex.what());
        }
    }();

    if (fail) FAIL() << "An error should have been thrown.";
}
