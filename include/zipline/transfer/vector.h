#pragma once

#include "base.h"

#include <vector>

namespace zipline {
    template <typename Socket, typename T>
    struct transfer<Socket, std::vector<T>> {
        static auto read(Socket& socket) -> std::vector<T> {
            TIMBER_TRACE("read vector");
            return read_sequence<Socket, std::vector<T>>(socket);
        }

        static auto write(
            Socket& socket,
            const std::vector<T>& vector
        ) -> void {
            TIMBER_TRACE("write vector");
            write_sequence(socket, vector);
        }
    };
}
