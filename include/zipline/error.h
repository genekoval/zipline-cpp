#pragma once

#include <zipline/transfer/transfer.h>

#include <stdexcept>

namespace zipline {
    template <typename Socket>
    struct zipline_error_base : std::runtime_error {
        zipline_error_base(const std::string& message) :
            std::runtime_error(message)
        {}

        virtual auto write(Socket& socket) const -> void {}
    };

    template <typename Socket, typename Derived>
    struct zipline_error : zipline_error_base<Socket> {
        zipline_error(const std::string& message) :
            zipline_error_base<Socket>(message)
        {}

        auto write(Socket& socket) const -> void final {
            transfer<Socket, Derived>::write(
                socket,
                *(static_cast<const Derived*>(this))
            );
        }
    };
}
