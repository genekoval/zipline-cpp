#pragma once

#include "base.h"

#include <ios>
#include <optional>

namespace zipline {
    template <typename Socket, typename T>
    struct coder<Socket, std::optional<T>> {
        static auto decode(Socket& socket) -> ext::task<std::optional<T>> {
            const auto has_value = co_await coder<Socket, bool>::decode(socket);

            TIMBER_TRACE(
                "decode optional: {}",
                has_value ? "has value" : "no value"
            );

            if (has_value) co_return co_await coder<Socket, T>::decode(socket);
            co_return std::nullopt;
        }

        static auto encode(
            Socket& socket,
            const std::optional<T>& optional
        ) -> ext::task<> {
            const auto has_value = optional.has_value();

            co_await coder<Socket, bool>::encode(socket, has_value);

            TIMBER_TRACE(
                "encode optional: {}",
                optional.has_value() ? "has value" : "no value"
            );

            if (has_value) {
                co_await coder<Socket, T>::encode(socket, optional.value());
            }
        }
    };
}
