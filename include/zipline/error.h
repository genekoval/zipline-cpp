#pragma once

#include <zipline/coder/coder.h>

#include <stdexcept>

namespace zipline {
    struct eof : std::runtime_error {
        eof() : std::runtime_error("unexpected EOF") {}
    };

    template <typename Socket>
    struct zipline_error_base : std::runtime_error {
        zipline_error_base(const std::string& message) :
            std::runtime_error(message)
        {}

        virtual ~zipline_error_base() {}

        virtual auto write(Socket& socket) const -> ext::task<> {
            co_return;
        }
    };

    template <typename Socket, typename Derived>
    struct zipline_error : zipline_error_base<Socket> {
        zipline_error(const std::string& message) :
            zipline_error_base<Socket>(message)
        {}

        virtual ~zipline_error() {}

        auto write(Socket& socket) const -> ext::task<> final {
            co_await coder<Socket, Derived>::encode(
                socket,
                *(static_cast<const Derived*>(this))
            );
        }
    };
}
