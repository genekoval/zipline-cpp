#pragma once

#include <zipline/response.h>

#include <string>
#include <string_view>
#include <timber/timber>
#include <type_traits>

namespace zipline {
    template <typename Socket, typename ErrorList>
    class protocol {
    protected:
        template <typename T>
        using response_type = zipline::response<T, Socket, ErrorList>;

        Socket* sock;
    public:
        const ErrorList* errors;

        protocol() : sock(nullptr), errors(nullptr) {}

        protocol(Socket& sock, const ErrorList& errors) :
            sock(&sock),
            errors(&errors)
        {}

        template <typename T>
        auto read() const -> ext::task<T> {
            co_return co_await coder<Socket, T>::decode(*sock);
        }

        template <typename T>
        auto response() const -> ext::task<T> {
            co_await sock->flush();
            auto res = response_type<T>(*sock);
            co_return co_await res.read(*errors);
        }

        template <typename T>
        auto write(const T& t) const -> ext::task<> {
            co_await coder<Socket, T>::encode(*sock, t);
        }

        auto write_bytes(std::span<const std::byte> bytes) -> ext::task<> {
            co_await sock->write(bytes.data(), bytes.size());
        }
    };
}
