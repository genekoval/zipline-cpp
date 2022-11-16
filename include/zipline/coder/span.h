#pragma once

#include "base.h"

#include <span>

namespace zipline {
    template <typename Socket, typename T>
    struct coder<Socket, std::span<T>> {
        static auto encode(
            Socket& socket,
            const std::span<T>& span
        ) -> ext::task<> {
            TIMBER_TRACE("write span");
            co_await encode_sequence(socket, span);
        }
    };
}
