#include "socket.test.hpp"

#include <cstring>

using zipline::test::buffer;

namespace {
    using namespace std::literals;

    class custom_error : public zipline::zipline_error {
        int n;
    public:
        custom_error(int n, std::string message) :
            zipline::zipline_error(message),
            n(n)
        {}

        auto number() const -> int {
            return n;
        }

        auto encode(
            zipline::io::abstract_writer& writer
        ) const -> ext::task<> override {
            co_await zipline::encode(number(), writer);
            co_await zipline::encode(what(), writer);
        }
    };

    struct unregistered_error : std::exception {
        auto what() const noexcept -> const char* override {
            return "unregistered error";
        }
    };

    constexpr auto number = 500;

    struct test_context {
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
            throw unregistered_error();
        }

        auto throw_custom_error(std::string what) -> ext::task<> {
            throw custom_error(number, what);
        }
    };

    using client = zipline::client<buffer, int>;

    using protocol = zipline::server_protocol<test_context, buffer>;
}

namespace zipline {
    template <>
    struct decoder<custom_error, io::abstract_reader> {
        static auto decode(
            io::abstract_reader& reader
        ) -> ext::task<custom_error> {
            const auto n = co_await zipline::decode<int>(reader);
            const auto message = co_await zipline::decode<std::string>(reader);

            throw custom_error(n, message);
        }
    };
}

namespace {
    using error_list = zipline::error_list<custom_error>;
}

class ServerProtocolTest : public SocketTestBase {
protected:
    const zipline::error_codes& codes = error_list::codes();
    const zipline::error_thrower& thrower = error_list::thrower();
    test_context context;
    protocol proto = protocol(context, codes, buffer);
    zipline::client<zipline::test::buffer&, int> client = {thrower, buffer};
};

TEST_F(ServerProtocolTest, UseWithReturn) {
    [this]() -> ext::detached_task {
        const auto numbers = std::vector<std::int32_t> { 1, 2, 3 };
        const auto expected = std::vector<std::int32_t> { 2, 4, 6 };

        for (const std::int32_t n : numbers) co_await client.write_all(n);

        co_await proto.use(&test_context::multiply_by_two);
        const auto result = co_await client.read_response<std::vector<int>>();

        EXPECT_EQ(expected, result);
    }();
}

TEST_F(ServerProtocolTest, UseWithVoid) {
    [this]() -> ext::detached_task {
        constexpr auto expected = "foo500";

        const auto one = "foo"s;
        const std::int32_t two = 500;

        co_await client.write_all(one, two);

        co_await proto.use(&test_context::concat);
        co_await client.read_response();

        EXPECT_EQ(expected, context.result);
    }();
}

TEST_F(ServerProtocolTest, UseWithThrow) {
    [&]() -> ext::detached_task {
        co_await proto.use(&test_context::throw_error);

        EXPECT_THROW(co_await client.read_response(), zipline::internal_error);
    }();
}

TEST_F(ServerProtocolTest, UseWithCustomThrow) {
    auto fail = false;
    [&]() -> ext::detached_task {
        constexpr auto what = "custom error message"sv;

        co_await client.write_all(what);
        co_await proto.use(&test_context::throw_custom_error);

        try {
            co_await client.read_response();
        }
        catch (const custom_error& ex) {
            EXPECT_EQ(number, ex.number());
            EXPECT_EQ(what, ex.what());
        }
    }();

    if (fail) FAIL() << "An error should have been thrown.";
}
