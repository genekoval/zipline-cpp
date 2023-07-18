#include "socket.test.hpp"

using zipline::error_list;
using zipline::test::buffer;
using zipline::zipline_error;

using abstract_reader = zipline::io::abstract_reader_wrapper<buffer>;
using abstract_writer = zipline::io::abstract_writer_wrapper<buffer>;

using namespace std::literals;

namespace {
    struct custom_error_1 : zipline_error {
        custom_error_1() : runtime_error("custom error 1") {}
    };

    struct custom_error_2 : zipline_error {
        custom_error_2() : runtime_error("custom error 2") {}
    };
}

namespace zipline {
    template <>
    struct decoder<custom_error_1, io::abstract_reader> {
        static auto decode(
            io::abstract_reader& reader
        ) -> ext::task<custom_error_1> {
            throw custom_error_1();
        }
    };

    template <>
    struct decoder<custom_error_2, io::abstract_reader> {
        static auto decode(
            io::abstract_reader& reader
        ) -> ext::task<custom_error_2> {
            throw custom_error_2();
        }
    };
}

class ErrorTest : public SocketTestBase {
protected:
    abstract_reader reader = buffer;
    abstract_writer writer = buffer;
};

TEST_F(ErrorTest, EmptyList) {
    ASSERT_EQ(0, error_list<>::size());
}

TEST_F(ErrorTest, OneError) {
    using errors = error_list<zipline_error>;

    const auto message = "basic error test"s;

    ASSERT_EQ(1, errors::size());

    const auto& codes = errors::codes();

    const auto ex = zipline_error(message);
    ASSERT_EQ(0, codes.code(ex));

    const auto& thrower = errors::thrower();
    auto thrown = false;

    [&]() -> ext::detached_task {
        co_await ex.encode(writer);

        try {
            co_await thrower.throw_error(0, reader);
        }
        catch (const zipline_error& ex) {
            thrown = true;
            EXPECT_EQ(message, ex.what());
        }
    }();

    if (!thrown) FAIL() << "Error should have been thrown.";
}

TEST_F(ErrorTest, MultipleErrors) {
    using errors = error_list<
        custom_error_1,
        custom_error_2
    >;

    ASSERT_EQ(2, errors::size());

    const auto& codes = errors::codes();
    const auto& thrower = errors::thrower();

    auto thrown = false;

    [&]() -> ext::detached_task {
        try {
            co_await thrower.throw_error(0, reader);
        }
        catch (const custom_error_1& ex) {
            thrown = true;
            EXPECT_EQ(0, codes.code(ex));
        }
    }();

    if (!thrown) FAIL() << "Error should have been thrown.";
    thrown = false;

    [&]() -> ext::detached_task {
        try {
            co_await thrower.throw_error(1, reader);
        }
        catch (const custom_error_2& ex) {
            thrown = true;
            EXPECT_EQ(1, codes.code(ex));
        }
    }();

    if (!thrown) FAIL() << "Error should have been thrown.";
}
