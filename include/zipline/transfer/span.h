#pragma once

#include "base.h"

#include <span>

namespace zipline {
    template <typename Socket, typename T>
    struct transfer<Socket, std::span<T>> {
        static auto read(Socket& socket) -> std::span<T> {
            ERROR() << "span does not support reading";
            throw unsupported_transfer_type();
        }

        static auto write(
            Socket& socket,
            const std::span<T>& span
        ) -> void {
            TRACE() << "write span";
            write_sequence(socket, span);
        }
    };
}
