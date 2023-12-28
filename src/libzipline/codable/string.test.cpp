#include "coder.test.hpp"

using namespace std::literals;

using StringTest = CoderTest;

TEST_F(StringTest, EmptyString) { test(std::string()); }

TEST_F(StringTest, String) { test(std::string("Hello, World!")); }

TEST_F(StringTest, StringView) {
    [&]() -> ext::detached_task {
        constexpr auto string = "Hello"sv;

        co_await zipline::encode(string, buffer);
        const auto value = co_await zipline::decode<std::string>(buffer);
        const auto view = std::string_view(value);

        EXPECT_EQ(string, view);
    }();
}

TEST_F(StringTest, CString) {
    [&]() -> ext::detached_task {
        constexpr auto string = "FooBar";

        co_await zipline::encode(string, buffer);
        const auto value = co_await zipline::decode<std::string>(buffer);

        EXPECT_EQ(std::string_view(string), std::string_view(value));
    }();
}
