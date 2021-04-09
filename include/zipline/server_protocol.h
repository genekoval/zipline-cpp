#pragma once

#include <zipline/protocol.h>

#include <tuple>

namespace zipline {
    template <
        typename Context,
        typename Socket,
        typename ErrorHandler = default_error_handler<Socket>
    >
    class server_protocol : public protocol<Socket, ErrorHandler> {
        Context* context;

        template <typename Tuple, std::size_t ...I>
        auto read_arg_sequence(
            Tuple& tuple,
            std::index_sequence<I...>
        ) const -> void {
            ((std::get<I>(tuple) =
                this->template read<std::tuple_element_t<I, Tuple>>()), ...);
        }

        template <typename ...Args>
        auto read_args() const -> std::tuple<Context*, Args...> {
            auto args = std::tuple<Args...>();
            read_arg_sequence(args, std::index_sequence_for<Args...>());

            return std::tuple_cat(std::make_tuple(context), args);
        }
    public:
        server_protocol(Context& context, Socket& socket) :
            protocol<Socket, ErrorHandler>(socket),
            context(&context)
        {}

        template <typename R, typename ...Args>
        auto use(R (*callable)(Context*, Args...)) const -> void {
            this->reply(std::apply(callable, read_args<Args...>()));
        }

        template <typename ...Args>
        auto use(void (*callable)(Context*, Args...)) const -> void {
            std::apply(callable, read_args<Args...>());
            this->reply();
        }
    };
}
