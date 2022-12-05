#pragma once

#include "io.hpp"

namespace zipline::io {
    struct null {
        auto read(void* dest, std::size_t len) -> ext::task<std::size_t>;

        auto write(const void* data, std::size_t len) -> ext::task<std::size_t>;
    };

    static_assert(source<null>);
    static_assert(sink<null>);
}
