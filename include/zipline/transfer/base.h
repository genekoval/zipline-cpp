#pragma once

#include <timber/timber>

namespace zipline {
    class unsupported_transfer_type : public std::runtime_error {
    public:
        unsupported_transfer_type() : std::runtime_error(
            "type cannot be transferred"
        ) {}
    };

    template <typename Socket, typename T>
    struct transfer {
        static auto read(Socket& socket) -> T {
            auto t = T();
            socket.read(&t, sizeof(T));
            return t;
        }

        static auto write(Socket& socket, const T& t) -> void {
            socket.write(&t, sizeof(T));
        }
    };

    template <typename Socket>
    auto read_size(Socket& socket) -> std::size_t {
        return transfer<Socket, std::size_t>::read(socket);
    };

    template <typename Socket>
    auto write_size(Socket& socket, std::size_t size) -> void {
        transfer<Socket, std::size_t>::write(socket, size);
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto read_array(Socket& socket, const T& value = T()) -> Container {
        const auto size = read_size(socket);
        TIMBER_TRACE("read array size: {}", size);

        auto container = Container(size, value);
        socket.read(container.data(), sizeof(T) * size);

        return container;
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto write_array(Socket& socket, const Container& container) -> void {
        const auto size = container.size();
        write_size(socket, size);
        TIMBER_TRACE("write array size: {}", size);

        socket.write(container.data(), sizeof(T) * size);
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto read_sequence(Socket& socket) -> Container {
        const auto size = read_size(socket);
        TIMBER_TRACE("read sequence size: {}", size);

        auto container = Container();

        for (std::size_t i = 0; i < size; ++i) {
            TIMBER_TRACE("read sequence item [{}]", i);
            container.push_back(transfer<Socket, T>::read(socket));
        }

        return container;
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type
    >
    auto write_sequence(Socket& socket, const Container& container) -> void {
        const auto size = container.size();
        write_size(socket, size);
        TIMBER_TRACE("write sequence size: {}", size);

        for (std::size_t i = 0; i < size; ++i) {
            TIMBER_TRACE("write sequence item [{}]", i);
            transfer<Socket, T>::write(socket, container[i]);
        }
    }
}
