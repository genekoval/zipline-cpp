#pragma once

#include <zipline/error.h>
#include <zipline/error_list.h>
#include <zipline/transfer.h>

namespace zipline {
    template <
        typename T,
        typename Socket,
        typename ErrorList
    >
    class response {
        using error = zipline_error_base<Socket>;

        Socket* sock;

        template <typename U>
        auto r() const -> U {
            return transfer<Socket, U>::read(*sock);
        }

        template <typename U>
        auto w(const U& t) const -> void {
            transfer<Socket, U>::write(*sock, t);
        }

        auto write_error(const std::exception& ex) const -> void {
            w<status_type>(1);
            w<std::string>(ex.what());
        }

        auto write_success() const -> void {
            w<status_type>(0);
        }
    public:
        response(Socket& sock) : sock(&sock) {}

        auto read(const ErrorList& errors) const -> T {
            const auto status = r<status_type>();

            switch (status) {
                case 0:
                    if constexpr (!std::is_void_v<T>) return r<T>();
                    break;
                case 1:
                    throw std::runtime_error(r<std::string>());
                default:
                    errors.throw_error(*sock, status - 2);
            }
        }

        template <typename Callable, typename Arguments>
        auto write(
            const ErrorList& errors,
            Callable&& callable,
            Arguments&& args
        ) const -> void {
            try {
                if constexpr (std::is_void_v<T>) {
                    std::apply(callable, args);
                    write_success();
                } else {
                    const auto result = std::apply(callable, args);
                    write_success();
                    w<T>(result);
                }
            }
            catch (const error& ex) {
                const auto code = errors.code(ex);
                w<status_type>(code + 2);
                ex.write(*sock);
            }
            catch (const std::exception& ex) {
                write_error(ex);
            }
        }
    };

    template <typename T, typename Socket, typename ErrorList>
    struct transfer<Socket, response<T, Socket, ErrorList>> {
        using type = response<T, Socket, ErrorList>;

        static auto read(Socket& sock) -> type {
            return type(sock);
        }
    };
}
