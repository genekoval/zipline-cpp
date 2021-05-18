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

        protocol(Socket& sock, const ErrorList& errors) :
            sock(&sock),
            errors(&errors)
        {}

        template <typename T>
        auto read() const -> std::remove_reference_t<T> {
            return transfer<Socket, std::remove_reference_t<T>>::read(*sock);
        }

        template <typename T>
        auto response() const -> T {
            sock->flush();
            auto res = response_type<T>(*sock);
            return res.read(*errors);
        }

        template <typename T>
        auto write(const T& t) const -> void {
            transfer<Socket, T>::write(*sock, t);
        }

        auto write_bytes(std::span<const std::byte> bytes) -> void {
            sock->write(bytes.data(), bytes.size());
        }
    };
}
