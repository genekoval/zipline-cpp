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
        ) const -> ext::task<> {
            ((std::get<I>(tuple) =
                co_await this->template read<std::tuple_element_t<I, Args>>()),
                ...
            );
        }

        template <typename ...Args>
        auto read_args() const -> ext::task<std::tuple<context_ref, Args...>> {
            auto args = std::tuple<Args...>();
            co_await read_arg_sequence(
                args,
                std::index_sequence_for<Args...>()
            );

            co_return std::tuple_cat(context, args);
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
        auto use(
            ext::task<R> (Context::* callable)(Args...)
        ) const -> ext::task<> {
            auto res = response<R, Socket, ErrorList>(*(this->sock));
            co_await res.write(
                *(this->errors),
                callable,
                co_await read_args<Args...>()
            );
        }
    };
}
