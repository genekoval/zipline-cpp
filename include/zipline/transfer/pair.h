#pragma once

#include "base.h"

#include <utility>

namespace zipline {
    template <typename Socket, typename T1, typename T2>
    struct transfer<Socket, std::pair<T1, T2>> {
        static auto read(Socket& socket) -> std::pair<T1, T2> {
            TIMBER_TRACE("reading pair");

            auto t1 = transfer<Socket, T1>::read(socket);
            auto t2 = transfer<Socket, T2>::read(socket);

            return std::make_pair(std::move(t1), std::move(t2));
        }

        static auto write(
            Socket& socket,
            const std::pair<T1, T2>& pair
        ) -> void {
            TIMBER_TRACE("write pair");

            transfer<Socket, T1>::write(socket, std::get<0>(pair));
            transfer<Socket, T2>::write(socket, std::get<1>(pair));
        }
    };
}
