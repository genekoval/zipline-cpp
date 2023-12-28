#pragma once

#include "io/io.hpp"
#include "response.hpp"

#include <span>

namespace zipline {
    template <typename Inner, typename EventT>
    requires io::reader<Inner> && io::writer<Inner> && codable<EventT, Inner>
    struct client {
        const error_thrower& errors;

        Inner inner;

        client() : errors(error_list<>::thrower()) {}

        template <typename... Args>
        client(const error_thrower& errors, Args&&... args) :
            errors(errors),
            inner(std::forward<Args>(args)...) {}

        auto await_write() -> ext::task<> { return inner.await_write(); }

        auto fill_buffer() -> ext::task<bool> { return inner.fill_buffer(); }

        auto flush() -> ext::task<> { return inner.flush(); }

        auto read(std::size_t len) -> ext::task<std::span<const std::byte>> {
            return inner.read(len);
        }

        auto read(void* dest, std::size_t len) -> ext::task<> {
            return inner.read(dest, len);
        }

        template <typename T>
        auto read() {
            return zipline::decode<T>(*this);
        }

        template <typename T = void>
        auto read_response() -> ext::task<T> {
            using response = zipline::response<T, client<Inner, EventT>>;

            co_await flush();
            const auto res = response(errors, *this);
            co_return co_await res.read();
        }

        template <typename R, typename... Args>
        auto send(EventT event, const Args&... args) -> ext::task<R> {
            co_await start(event, args...);
            co_return co_await read_response<R>();
        }

        template <typename... T>
        auto start(EventT event, const T&... args) -> ext::task<> {
            co_await write_all(event, args...);
        }

        auto try_write(const void* src, std::size_t len) -> std::size_t {
            return inner.try_write(src, len);
        }

        auto write(const void* src, std::size_t len) -> ext::task<> {
            return inner.write(src, len);
        }

        template <typename... T>
        auto write_all(const T&... t) -> ext::task<> {
            (co_await zipline::encode(t, *this), ...);
        }
    };
}
