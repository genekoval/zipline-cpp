#pragma once

#include "response.hpp"

#include <string>
#include <string_view>
#include <timber/timber>
#include <type_traits>

namespace zipline {
    template <typename Socket, typename ErrorList>
    requires io::reader<Socket> && io::writer<Socket>
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
        requires decodable<T, Socket>
        auto read() const -> ext::task<std::remove_cvref_t<T>> {
            return decode<T>(*sock);
        }

        template <typename T>
        auto response() const -> ext::task<std::remove_cvref_t<T>> {
            co_await sock->flush();
            auto res = response_type<std::remove_cvref_t<T>>(*sock);
            co_return co_await res.read(*errors);
        }

        template <typename T>
        requires encodable<T, Socket>
        auto write(const T& t) const -> ext::task<> {
            return encode(t, *sock);
        }
    };
}
