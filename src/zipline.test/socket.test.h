#pragma once

#include <cstddef>
#include <cstring>
#include <gtest/gtest.h>

namespace zipline::test {
    class socket {
        std::byte* buffer;
        std::size_t head = 0;
        std::size_t tail = 0;
    public:
        socket(std::byte* buffer);

        auto flush() -> void;

        auto read(void* dest, std::size_t len) -> std::size_t;

        auto write(const void* src, std::size_t len) -> std::size_t;
    };
}

class SocketTestBase : public testing::Test {
    std::array<std::byte, 1024> buffer;
protected:
    zipline::test::socket sock = zipline::test::socket(buffer.data());
};
