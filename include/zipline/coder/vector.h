#pragma once

#include "base.h"

#include <vector>

namespace zipline {
    template <typename Socket, typename T>
    struct coder<Socket, std::vector<T>> {
        static auto decode(Socket& socket) -> ext::task<std::vector<T>> {
            TIMBER_TRACE("read vector");
            co_return co_await decode_sequence<Socket, std::vector<T>>(socket);
        }

        static auto encode(
            Socket& socket,
            const std::vector<T>& vector
        ) -> ext::task<> {
            TIMBER_TRACE("write vector");
            co_await encode_sequence(socket, vector);
        }
    };
}
