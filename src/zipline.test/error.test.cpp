#include <zipline/zipline>

#include <gtest/gtest.h>

#define CUSTOM_ERROR(n) \
    struct custom_error_##n :\
        zipline::zipline_error<socket, custom_error_##n>\
    {\
        custom_error_##n() :\
            zipline::zipline_error<socket, custom_error_##n>(\
                "custom error " #n)\
        {}\
    };

#define CUSTOM_ERROR_TRANSFER(n) \
    template <>\
    struct coder<socket, custom_error_##n> {\
        static auto decode(socket& sock) -> ext::task<custom_error_##n> {\
            co_return custom_error_##n();\
        }\
        static auto encode(socket& sock, custom_error_##n ex) -> ext::task<> {\
            co_return;\
        }\
    };

using namespace std::literals;

struct socket {};

CUSTOM_ERROR(1)
CUSTOM_ERROR(2)

namespace zipline {
    CUSTOM_ERROR_TRANSFER(1)
    CUSTOM_ERROR_TRANSFER(2)
}

template <typename ...Errors>
using error_list = zipline::error_list<socket, Errors...>;

TEST(ErrorTest, EmptyList) {
    const auto errors = error_list<>();
    ASSERT_EQ(0, errors.size());
}

TEST(ErrorTest, OneError) {
    const auto errors = error_list<custom_error_1>();
    ASSERT_EQ(1, errors.size());

    const auto ex = custom_error_1();
    ASSERT_EQ(0, errors.code(ex));

    auto sock = socket();
    auto fail = false;

    [&]() -> ext::detached_task {
        try {
            co_await errors.throw_error(sock, 0);
            fail = true;
            co_return;
        }
        catch (const custom_error_1& ex) {
            EXPECT_EQ("custom error 1"sv, ex.what());
        }
    }();

    if (fail) FAIL() << "Error should have been thrown.";
}

TEST(ErrorTest, MultipleErrors) {
    const auto errors = error_list<
        custom_error_1,
        custom_error_2
    >();

    ASSERT_EQ(2, errors.size());

    auto sock = socket();
    auto fail = false;

    [&]() -> ext::detached_task {
        try {
            co_await errors.throw_error(sock, 0);
            fail = true;
            co_return;
        }
        catch (const custom_error_1& ex) {
            EXPECT_EQ(0, errors.code(ex));
        }
    }();

    if (fail) FAIL() << "Error should have been thrown.";

    [&]() -> ext::detached_task {
        try {
            co_await errors.throw_error(sock, 1);
            fail = true;
            co_return;
        }
        catch (const custom_error_2& ex) {
            EXPECT_EQ(1, errors.code(ex));
        }
    }();

    if (fail) FAIL() << "Error should have been thrown.";
}
