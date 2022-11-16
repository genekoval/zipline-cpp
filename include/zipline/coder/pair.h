#pragma once

#include "base.h"

#include <utility>

namespace zipline {
    template <typename Socket, typename T1, typename T2>
    struct coder<Socket, std::pair<T1, T2>> {
        static auto decode(Socket& socket) -> ext::task<std::pair<T1, T2>> {
            TIMBER_TRACE("reading pair");

            auto t1 = co_await coder<Socket, T1>::decode(socket);
            auto t2 = co_await coder<Socket, T2>::decode(socket);

            co_return std::make_pair(std::move(t1), std::move(t2));
        }

        static auto encode(
            Socket& socket,
            const std::pair<T1, T2>& pair
        ) -> void {
            TIMBER_TRACE("write pair");

            co_await coder<Socket, T1>::encode(socket, std::get<0>(pair));
            co_await coder<Socket, T2>::encode(socket, std::get<1>(pair));
        }
    };
}
