#include <zipline/memory_buffer.h>

#include <cstring>

namespace zipline {
    memory_buffer::memory_buffer(std::byte* buffer) : buffer(buffer) {}

    auto memory_buffer::flush() -> ext::task<> { co_return; }

    auto memory_buffer::read(
        void* dest,
        std::size_t len
    ) -> ext::task<std::size_t> {
        std::memcpy(dest, &buffer[head], len);
        head += len;

        if (head == tail) {
            head = 0;
            tail = 0;
        }

        co_return len;
    }

    auto memory_buffer::write(
        const void* src,
        std::size_t len
    ) -> ext::task<std::size_t> {
        std::memcpy(&buffer[tail], src, len);
        tail += len;

        co_return len;
    }
}
