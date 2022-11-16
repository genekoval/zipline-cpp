#include <zipline/zipline>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
    constexpr auto buffer_size = 64;

    using buffered_socket =
        zipline::buffered_socket<zipline::memory_buffer, buffer_size>;
}

class BufferTest : public testing::Test {
    std::array<std::byte, 1024> buffer;
protected:
    buffered_socket socket =
        buffered_socket(zipline::memory_buffer(buffer.data()));
};

TEST_F(BufferTest, ReadWrite) {
    [&]() -> ext::detached_task {
        constexpr auto original = std::string_view("Hello, Buffer Test!");

        co_await socket.write(original.data(), original.size());
        co_await socket.flush();

        auto copy = std::array<char, original.size()>();
        co_await socket.read(copy.data(), original.size());

        const auto result = std::string_view(copy.data(), copy.size());

        EXPECT_EQ(original, result);
    }();
}
