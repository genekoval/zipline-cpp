#pragma once

#include <zipline/protocol.h>

#include <tuple>

namespace zipline {
    template <
        typename Context,
        typename Socket,
        typename ErrorList
    >
    class server_protocol : public protocol<Socket, ErrorList> {
        using context_ref = std::reference_wrapper<Context>;

        std::tuple<context_ref> context;

        template <typename Args, std::size_t ...I>
        auto read_arg_sequence(
            Args& tuple,
            std::index_sequence<I...>
        ) const -> void {
            ((std::get<I>(tuple) =
                this->template read<std::tuple_element_t<I, Args>>()), ...);
        }

        template <typename ...Args>
        auto read_args() const -> std::tuple<context_ref, Args...> {
            auto args = std::tuple<Args...>();
            read_arg_sequence(args, std::index_sequence_for<Args...>());

            return std::tuple_cat(context, args);
        }
    public:
        server_protocol(
            Context& context,
            Socket& socket,
            const ErrorList& errors
        ) :
            protocol<Socket, ErrorList>(socket, errors),
            context(std::make_tuple(std::ref(context)))
        {}

        template <typename R, typename ...Args>
        auto use(R (Context::* callable)(Args...)) const -> void {
            auto res = response<R, Socket, ErrorList>(*(this->sock));
            res.write(*(this->errors), callable, read_args<Args...>());
        }
    };
}
