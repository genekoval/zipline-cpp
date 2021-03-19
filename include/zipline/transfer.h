#pragma once

#include <optional>
#include <span>
#include <string>
#include <timber/timber>
#include <vector>

namespace zipline {
    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type,
        typename size_type = typename Container::size_type
    >
    auto read_array(Socket& sock, const T& value = T()) -> Container {
        auto size = size_type();
        sock.read(&size, sizeof(size_type));
        DEBUG() << "read array size: " << size;

        auto buffer = Container(size, value);
        sock.read(buffer.data(), sizeof(T) * size);

        return buffer;
    }

    template <
        typename Socket,
        typename Container,
        typename T = typename Container::value_type,
        typename size_type = typename Container::size_type
    >
    auto write_array(Socket& sock, const Container& container) -> void {
        const auto size = container.size();
        sock.write(&size, sizeof(size_type));
        DEBUG() << "write array size: " << size;

        sock.write(container.data(), sizeof(T) * size);
    }

    template <typename Socket, typename T>
    struct transfer {
        static auto read(Socket& sock) -> T {
            auto t = T();
            sock.read(&t, sizeof(T));
            return t;
        }

        static auto write(Socket& sock, const T& t) -> void {
            sock.write(&t, sizeof(T));
        }
    };

    template <typename Socket, typename T>
    struct transfer<Socket, std::optional<T>> {
        static auto read(Socket& sock) -> std::optional<T> {
            auto has_value = transfer<Socket, bool>::read(sock);

            if (has_value) return transfer<Socket, T>::read(sock);
            return {};
        }

        static auto write(
            Socket& sock,
            const std::optional<T> opt
        ) -> void {
            auto has_value = opt.has_value();
            transfer<Socket, bool>::write(sock, has_value);

            if (has_value) transfer<Socket, T>::write(sock, opt.value());
            else DEBUG() << "write optional: no value";
        }
    };

    template <typename Socket, typename T1, typename T2>
    struct transfer<Socket, std::pair<T1, T2>> {
        static auto read(Socket& sock) -> std::pair<T1, T2> {
            auto t1 = transfer<Socket, T1>::read(sock);
            auto t2 = transfer<Socket, T2>::read(sock);

            return std::make_pair(std::move(t1), std::move(t2));
        }

        static auto write(
            Socket& sock,
            const std::pair<T1, T2>& pair
        ) -> void {
            transfer<Socket, T1>::write(sock, std::get<0>(pair));
            transfer<Socket, T2>::write(sock, std::get<1>(pair));
        }
    };

    template <typename Socket, typename T>
    struct transfer<Socket, std::span<const T>> {
        static auto write(
            Socket& sock,
            const std::span<const T> span
        ) -> void {
            write_array<Socket, std::span<const T>>(sock, span);
        }
    };

    template <typename Socket>
    struct transfer<Socket, std::string> {
        static auto read(Socket& sock) -> std::string {
            const auto string = read_array<Socket, std::string>(sock, '\0');

            if (timber::reporting_level() >= timber::level::debug) {
                const auto subsize = std::min(string.size(), 25UL);
                const auto substr = string.substr(0, subsize);

                DEBUG()
                    << "read: " << substr
                    << (subsize < string.size()
                        ?
                            " ...(" +
                            std::to_string(string.size() - subsize) +
                            " more)..."
                        :
                            ""
                    );
            }

            return string;
        }

        static auto write(
            Socket& sock,
            const std::string& string
        ) -> void {
            write_array<Socket, std::string>(sock, string);
            DEBUG() << "write: " << string;
        }
    };

    template <typename Socket>
    struct transfer<Socket, std::string_view> {
        static auto write(
            Socket& sock,
            const std::string_view& string
        ) -> void {
            write_array<Socket, std::string_view>(sock, string);
            DEBUG() << "write: " << string;
        }
    };

    template <typename Socket, typename T>
    struct transfer<Socket, std::vector<T>> {
        static auto read(Socket& sock) -> std::vector<T> {
            using size_type = typename std::vector<T>::size_type;

            auto size = transfer<Socket, size_type>::read(sock);

            DEBUG() << "read vector size: " << size;

            auto container = std::vector<T>();

            for (size_type i = 0; i < size; ++i) {
                container.push_back(transfer<Socket, T>::read(sock));
            }

            return container;
        }

        static auto write(
            Socket& sock,
            const std::vector<T>& vector
        ) -> void {
            using size_type = typename std::vector<T>::size_type;

            const auto size = vector.size();
            transfer<Socket, size_type>::write(sock, size);
            DEBUG() << "write vector size: " << size;

            for (size_type i = 0; i < size; ++i) {
                DEBUG() << "write vector [" << i << "]";
                transfer<Socket, T>::write(sock, vector[i]);
            }
        }
    };
}
