#pragma once

#include <ext/coroutine>
#include <timber/timber>

namespace zipline {
    class unsupported_transfer_type : public std::runtime_error {
    public:
        unsupported_transfer_type() : std::runtime_error(
            "type cannot be transferred"
        ) {}
    };

    template <typename Socket, typename T>
    struct coder {
        static auto decode(Socket& socket) -> ext::task<T> {
            auto t = T();
            co_await socket.read(&t, sizeof(T));
            co_return t;
        }

        static auto encode(
            Socket& socket,
            const T& t
        ) -> ext::task<> {
            co_await socket.write(&t, sizeof(T));
        }
    };

    template <typename Socket>
    auto decode_size(Socket& socket) -> ext::task<std::size_t> {
        co_return co_await coder<Socket, std::size_t>::decode(socket);
    };

    template <typename Socket>
    auto encode_size(Socket& socket, std::size_t size) -> ext::task<> {
        co_await coder<Socket, std::size_t>::encode(socket, size);
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto decode_array(
        Socket& socket,
        const T& value = T()
    ) -> ext::task<Container> {
        const auto size = co_await decode_size(socket);
        TIMBER_TRACE("decode array size: {}", size);

        auto container = Container(size, value);
        co_await socket.read(container.data(), sizeof(T) * size);

        co_return container;
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto encode_array(
        Socket& socket,
        const Container& container
    ) -> ext::task<> {
        const auto size = container.size();
        co_await encode_size(socket, size);
        TIMBER_TRACE("encode array size: {}", size);

        co_await socket.write(container.data(), sizeof(T) * size);
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto decode_sequence(
        Socket& socket
    ) -> ext::task<Container> {
        const auto size = co_await decode_size(socket);
        TIMBER_TRACE("decode sequence size: {}", size);

        auto container = Container();

        for (auto i = std::size_t(0); i < size; ++i) {
            TIMBER_TRACE("decode sequence item [{}]", i);
            container.push_back(co_await coder<Socket, T>::decode(socket));
        }

        co_return container;
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto encode_sequence(
        Socket& socket,const Container& container) -> ext::task<> {
        const auto size = container.size();
        co_await encode_size(socket, size);
        TIMBER_TRACE("encode sequence size: {}", size);

        for (auto i = std::size_t(0); i < size; ++i) {
            TIMBER_TRACE("encode sequence item [{}]", i);
            co_await coder<Socket, T>::encode(socket, container[i]);
        }
    }
}
