#pragma once

#include "base.h"

#define ZIPLINE_OBJECT_TEMPLATE(Type, ...) \
    struct transfer<Socket, Type> { \
        using Object = zipline::object<Socket, Type>; \
        static auto read(Socket& socket) -> Type { \
            return Object::read(socket, __VA_ARGS__); \
        } \
        static auto write(Socket& socket, const Type& t) -> void { \
            Object::write(socket, t, __VA_ARGS__); \
        } \
    };


#define ZIPLINE_OBJECT(Type, ...) \
    template <typename Socket> \
    ZIPLINE_OBJECT_TEMPLATE(Type, __VA_ARGS__)

namespace zipline {
    template <typename Socket, typename T>
    class object {
        template <typename Type, typename Base>
        static auto read_member(
            Socket& socket,
            T& t,
            Type Base::* member
        ) -> void {
            t.*member = transfer<Socket, Type>::read(socket);
        }

        template <typename Type, typename Base>
        static auto write_member(
            Socket& socket,
            const T& t,
            Type Base::* member
        ) -> void {
            transfer<Socket, Type>::write(socket, t.*member);
        }
    public:
        template <typename ...Members>
        static auto read(Socket& socket, Members... members) -> T {
            auto t = T();
            (read_member(socket, t, members), ...);
            return t;
        }

        template <typename ...Members>
        static auto write(
            Socket& socket,
            const T& t,
            Members... members
        ) -> void {
            (write_member(socket, t, members), ...);
        }
    };
}
