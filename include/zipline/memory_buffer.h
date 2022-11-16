#pragma once

#include <cstddef>
#include <ext/coroutine>

namespace zipline {
    class memory_buffer {
        std::byte* buffer = nullptr;
        std::size_t head = 0;
        std::size_t tail = 0;
    public:
        memory_buffer() = default;

        explicit memory_buffer(std::byte* buffer);

        auto flush() -> ext::task<>;

        auto read(void* dest, std::size_t len) -> ext::task<std::size_t>;

        auto write(const void* src, std::size_t len) -> ext::task<std::size_t>;
    };
}
