#include "socket.test.h"

namespace zipline::test {
    socket::socket(std::byte* buffer) : buffer(buffer) {}

    auto socket::flush() -> void {}

    auto socket::read(void* dest, std::size_t len) -> std::size_t {
        std::memcpy(dest, &buffer[head], len);
        head += len;

        if (head == tail) {
            head = 0;
            tail = 0;
        }

        return len;
    }

    auto socket::write(const void* src, std::size_t len) -> std::size_t {
        std::memcpy(&buffer[tail], src, len);
        tail += len;

        return len;
    }
}
