#include <zipline/zipline>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::Test;
using zipline::io::array_buffer;

class BufferTest : public Test {
protected:
    array_buffer<64> buffer;
};

TEST_F(BufferTest, ReadWrite) {
    [&]() -> ext::detached_task {
        constexpr auto original = std::string_view("Hello, Buffer Test!");

        co_await buffer.write(original.data(), original.size());

        auto copy = std::array<char, original.size()>();
        co_await buffer.read(copy.data(), original.size());

        const auto result = std::string_view(copy.data(), copy.size());

        EXPECT_EQ(original, result);
    }();
}
