#pragma once

#include "base.h"

#include <ios>
#include <optional>

namespace zipline {
    template <typename Socket, typename T>
    struct transfer<Socket, std::optional<T>> {
        static auto read(Socket& socket) -> std::optional<T> {
            auto has_value = transfer<Socket, bool>::read(socket);

            TIMBER_TRACE("read optional: contains value: {}", has_value);

            if (has_value) return transfer<Socket, T>::read(socket);
            return {};
        }

        static auto write(
            Socket& socket,
            const std::optional<T>& optional
        ) -> void {
            auto has_value = optional.has_value();

            transfer<Socket, bool>::write(socket, has_value);

            TIMBER_TRACE(
                "write optional: contains value: {}",
                optional.has_value()
            );

            if (has_value) transfer<Socket, T>::write(socket, optional.value());
        }
    };
}
