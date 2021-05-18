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
    struct transfer<socket, custom_error_##n> {\
        static auto read(socket& sock) -> custom_error_##n {\
            return custom_error_##n();\
        }\
        static auto write(socket& sock, custom_error_##n ex) -> void {}\
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

    try {
        errors.throw_error(sock, 0);
        FAIL() << "Error should have been thrown.";
    }
    catch (const custom_error_1& ex) {
        ASSERT_EQ("custom error 1"sv, ex.what());
    }
}

TEST(ErrorTest, MultipleErrors) {
    const auto errors = error_list<
        custom_error_1,
        custom_error_2
    >();

    ASSERT_EQ(2, errors.size());

    auto sock = socket();

    try {
        errors.throw_error(sock, 0);
        FAIL() << "Error should have been thrown.";
    }
    catch (const custom_error_1& ex) {
        ASSERT_EQ(0, errors.code(ex));
    }

    try {
        errors.throw_error(sock, 1);
        FAIL() << "Error should have been thrown.";
    }
    catch (const custom_error_2& ex) {
        ASSERT_EQ(1, errors.code(ex));
    }
}
