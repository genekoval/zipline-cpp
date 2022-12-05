#pragma once

#include <ext/coroutine>
#include <span>

namespace zipline::io {
    template <typename T>
    concept source = requires(T& t, void* dest, std::size_t len) {
        { t.read(dest, len) } -> std::same_as<ext::task<std::size_t>>;
    };

    template <typename T>
    concept sink = requires(T& t, const void* data, std::size_t len) {
        { t.write(data, len) } -> std::same_as<ext::task<std::size_t>>;
    };

    template <typename T>
    concept reader = requires(T& t, void* dest, std::size_t len) {
        { t.read(len) }-> std::same_as<ext::task<std::span<const std::byte>>>;
        { t.read(dest, len) } -> std::same_as<ext::task<>>;
    };

    template <typename T>
    concept writer = requires(T& t, const void* src, std::size_t len) {
        { t.flush() } -> std::same_as<ext::task<>>;
        { t.write(src, len) } -> std::same_as<ext::task<>>;
    };
}
