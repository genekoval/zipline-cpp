#pragma once

#include <optional>
#include <string>

namespace zipline {
    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type,
        typename size_type = typename Container::size_type
    >
    auto read_array(const Socket& sock, const T& value = T()) -> Container {
        auto size = size_type();
        sock.recv(&size, sizeof(size_type));

        auto buffer = Container(size, value);
        sock.recv(buffer.data(), sizeof(T) * size);

        return buffer;
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type,
        typename size_type = typename Container::size_type
    >
    auto write_array(const Socket& sock, const Container& container) -> void {
        const auto size = container.size();
        sock.send(&size, sizeof(size_type));
        sock.send(container.data(), sizeof(T) * size);
    }

    template <typename Socket, typename T>
    struct transfer {
        static auto read(const Socket& sock) -> T {
            auto t = T();
            sock.recv(&t, sizeof(T));
            return t;
        }

        static auto write(const Socket& sock, const T& t) -> void {
            sock.send(&t, sizeof(T));
        }
    };

    template <typename Socket, typename T>
    struct transfer<Socket, std::optional<T>> {
        static auto read(const Socket& sock) -> std::optional<T> {
            auto has_value = transfer<Socket, bool>::read(sock);

            if (has_value) return transfer<Socket, T>::read(sock);
            return {};
        }

        static auto write(
            const Socket& sock,
            const std::optional<T> opt
        ) -> void {
            auto has_value = opt.has_value();
            transfer<Socket, bool>::write(sock, has_value);

            if (has_value) transfer<Socket, T>::write(sock, opt.value());
        }
    };

    template <typename Socket>
    struct transfer<Socket, std::string> {
        static auto read(const Socket& sock) -> std::string {
            return read_array<Socket, std::string>(sock, '\0');
        }

        static auto write(
            const Socket& sock,
            const std::string& string
        ) -> void {
            write_array<Socket, std::string>(sock, string);
        }
    };

    template <typename Socket>
    struct transfer<Socket, std::string_view> {
        static auto write(
            const Socket& sock,
            const std::string_view& string
        ) -> void {
            write_array<Socket, std::string_view>(sock, string);
        }
    };
}
