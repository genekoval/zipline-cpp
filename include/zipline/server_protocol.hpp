#pragma once

#include "response.hpp"

#include <tuple>

namespace zipline {
    template <typename Context, typename Inner>
    requires io::reader<Inner> && io::writer<Inner>
    class server_protocol {
        using context_ref = std::reference_wrapper<Context>;

        std::tuple<context_ref> context;
        const error_codes& errors;
        Inner& inner;

        template <typename Args, std::size_t ...I>
        auto read_arg_sequence(
            Args& tuple,
            std::index_sequence<I...>
        ) const -> ext::task<> {
            ((std::get<I>(tuple) =
                co_await zipline::decode<std::tuple_element_t<I, Args>>(inner)
            ),...);
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
            const error_codes& errors,
            Inner& inner
        ) :
            context(std::make_tuple(std::ref(context))),
            errors(errors),
            inner(inner)
        {}

        auto fill_buffer() -> ext::task<bool> {
            return inner.fill_buffer();
        }

        template <typename T>
        requires decodable<T, Inner>
        auto read() const {
            return zipline::decode<T>(inner);
        }

        template <typename R, typename ...Args>
        auto use(
            ext::task<R> (Context::* callable)(Args...)
        ) const -> ext::task<> {
            const auto res = responder<R, Inner>(errors, inner);
            auto args = co_await read_args<Args...>();

            co_await res.write(callable, std::move(args));
        }
    };
}
