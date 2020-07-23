#pragma once

#include <string>
#include <string_view>

namespace zipline {
    template <typename Socket>
    class protocol {
        const Socket* sock;
    public:
        using socket_type = Socket;

        protocol(const Socket& sock) : sock(&sock) {}

        template <typename T>
        auto read() const -> T {
            auto t = T();
            sock->recv(&t, sizeof(T));
            return t;
        }

        template <
            typename Container,
            typename T = typename Container::value_type,
            typename size_type = typename Container::size_type
        >
        auto read_array(const T& value = T()) const -> Container {
            auto size = read<size_type>();
            auto buffer = Container(size, value);

            sock->recv(buffer.data(), sizeof(T) * size);

            return buffer;
        }

        auto read_string() const -> std::string {
            return read_array<std::string>('\0');
        }

        template <typename T>
        auto write(const T& t) const -> void {
            sock->send(&t, sizeof(T));
        }

        template <
            typename Container,
            typename T = typename Container::value_type
        >
        auto write_array(const Container& container) const -> void {
            const auto size = container.size();
            write(size);
            sock->send(container.data(), sizeof(T) * size);
        }

        auto write_string(std::string_view string) const -> void {
            write_array(string);
        }
    };
}
