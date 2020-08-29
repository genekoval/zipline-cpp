#pragma once

#include <optional>
#include <span>
#include <string>
#include <vector>

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

    template <typename Socket, typename T1, typename T2>
    struct transfer<Socket, std::pair<T1, T2>> {
        static auto read(const Socket& sock) -> std::pair<T1, T2> {
            auto t1 = transfer<Socket, T1>::read(sock);
            auto t2 = transfer<Socket, T2>::read(sock);

            return std::make_pair(std::move(t1), std::move(t2));
        }

        static auto write(
            const Socket& sock,
            const std::pair<T1, T2>& pair
        ) -> void {
            transfer<Socket, T1>::write(sock, std::get<0>(pair));
            transfer<Socket, T2>::write(sock, std::get<1>(pair));
        }
    };

    template <typename Socket, typename T>
    struct transfer<Socket, std::span<const T>> {
        static auto write(
            const Socket& sock,
            const std::span<const T> span
        ) -> void {
            write_array<Socket, std::span<const T>>(sock, span);
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

    template <typename Socket, typename T>
    struct transfer<Socket, std::vector<T>> {
        static auto read(const Socket& sock) -> std::vector<T> {
            return read_array<Socket, std::vector<T>>(sock);
        }

        static auto write(
            const Socket& sock,
            const std::vector<T>& vector
        ) -> void {
            write_array<Socket, std::vector<T>>(sock, vector);
        }
    };
}
