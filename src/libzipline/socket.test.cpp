#include "socket.test.hpp"

namespace zipline::test {
    buffer::buffer() : storage(1024) {}

    auto buffer::await_write() -> ext::task<> { co_return; }

    auto buffer::empty() const noexcept -> bool { return storage.empty(); }

    auto buffer::fill_buffer() -> ext::task<bool> { co_return true; }

    auto buffer::flush() -> ext::task<> { co_return; }

    auto buffer::read(std::size_t len)
        -> ext::task<std::span<const std::byte>> {
        co_return storage.read(len);
    }

    auto buffer::read(void* dest, std::size_t len) -> ext::task<> {
        storage.read(dest, len);
        co_return;
    }

    auto buffer::try_write(const void* src, std::size_t len) -> std::size_t {
        storage.write(src, len);
        return len;
    }

    auto buffer::write(const void* src, std::size_t len) -> ext::task<> {
        storage.write(src, len);
        co_return;
    }
}
