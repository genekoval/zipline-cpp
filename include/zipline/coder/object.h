#pragma once

#include "base.h"

#define ZIPLINE_OBJECT_TEMPLATE(Type, ...) \
    struct coder<Socket, Type> { \
        using object_type = zipline::object<Socket, Type>; \
        static auto decode( \
            Socket& socket \
        ) -> ext::task<Type> { \
            co_return co_await object_type::decode(socket, __VA_ARGS__); \
        } \
        static auto encode( \
            Socket& socket, \
            const Type& t \
        ) -> ext::task<> { \
            co_await object_type::encode(socket, t, __VA_ARGS__); \
        } \
    };


#define ZIPLINE_OBJECT(Type, ...) \
    template <typename Socket> \
    ZIPLINE_OBJECT_TEMPLATE(Type, __VA_ARGS__)

namespace zipline {
    template <typename Socket, typename T>
    class object {
        template <typename Type, typename Base>
        static auto decode_member(
            Socket& socket,
            T& t,
            Type Base::* member
        ) -> ext::task<> {
            t.*member = co_await coder<Socket, Type>::decode(socket);
        }

        template <typename Type, typename Base>
        static auto encode_member(
            Socket& socket,
            const T& t,
            Type Base::* member
        ) -> ext::task<> {
            co_await coder<Socket, Type>::encode(socket, t.*member);
        }
    public:
        template <typename ...Members>
        static auto decode(Socket& socket, Members... members) -> ext::task<T> {
            auto t = T();
            (co_await decode_member(socket, t, members), ...);
            co_return t;
        }

        template <typename ...Members>
        static auto encode(
            Socket& socket,
            const T& t,
            Members... members
        ) -> ext::task<> {
            (co_await encode_member(socket, t, members), ...);
        }
    };
}
