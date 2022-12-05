#include <zipline/io/null.hpp>

namespace zipline::io {
    auto null::read(void* dest, std::size_t len) -> ext::task<std::size_t> {
        co_return 0;
    }

    auto null::write(
        const void* data,
        std::size_t len
    ) -> ext::task<std::size_t> {
        co_return len;
    }
}
