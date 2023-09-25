#pragma once

#include <zipline/zipline>

#include <gtest/gtest.h>
#include <netcore/netcore>

namespace zipline::test {
    class buffer {
        netcore::buffer storage;
    public:
        buffer();

        auto await_write() -> ext::task<>;

        auto empty() const noexcept -> bool;

        auto fill_buffer() -> ext::task<bool>;

        auto flush() -> ext::task<>;

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>>;

        auto read(void* dest, std::size_t len) -> ext::task<>;

        auto try_write(const void* src, std::size_t len) -> std::size_t;

        auto write(const void* src, std::size_t len) -> ext::task<>;
    };

    static_assert(io::reader<buffer>);
    static_assert(io::writer<buffer>);
}

class SocketTestBase : public testing::Test {
protected:
    zipline::test::buffer buffer;
};
