#pragma once

#include <zipline/error.h>
#include <zipline/error_list.h>
#include <zipline/coder/coder.h>

namespace zipline {
    template <
        typename T,
        typename Socket,
        typename ErrorList
    >
    class response_base {
    protected:
        Socket* socket;

        auto handle_error(
            std::exception_ptr ex,
            const ErrorList& errors
        ) const -> ext::task<> {
            auto code = status_type(0);
            auto task = ext::task<>();
            auto message = std::string_view();

            try {
                if (ex) std::rethrow_exception(ex);
            }
            catch (const zipline_error_base<Socket>& error) {
                code = errors.code(error) + 2;
                task = error.write(*socket);
            }
            catch (const std::exception& ex) {
                code = 1;
                message = ex.what();
                task =
                    coder<Socket, std::string_view>::encode(*socket, message);
            }

            co_await coder<Socket, status_type>::encode(*socket, code);
            co_await task;
        }

        template <typename U>
        auto decode() const -> ext::task<U> {
            co_return co_await coder<Socket, U>::decode(*socket);
        }
    public:
        response_base(Socket& socket) : socket(&socket) {}

        auto read(const ErrorList& errors) const -> ext::task<T> {
            const auto status = co_await decode<status_type>();

            switch (status) {
                case 0:
                    if constexpr (std::is_void_v<T>) co_return;
                    else co_return co_await decode<T>();
                case 1: {
                    const auto message = co_await decode<std::string>();
                    throw std::runtime_error(message);
                }
                default:
                    co_await errors.throw_error(*socket, status - 2);
            }

            __builtin_unreachable();
        }
    };

    template <typename T, typename Socket, typename ErrorList>
    struct response : response_base<T, Socket, ErrorList> {
        using response_base<T, Socket, ErrorList>::response_base;

        template <typename Callable, typename Arguments>
        auto write(
            const ErrorList& errors,
            Callable&& callable,
            Arguments&& args
        ) const -> ext::task<> {
            auto result = T();
            auto task = ext::task<>();

            try {
                result = co_await std::apply(callable, args);
                task = [](auto& socket, auto& result) -> ext::task<> {
                    co_await coder<Socket, status_type>::encode(socket, 0);
                    co_await coder<Socket, T>::encode(socket, result);
                }(*this->socket, result);
            }
            catch (...) {
                task = this->handle_error(std::current_exception(), errors);
            }

            co_await task;
            co_await this->socket->flush();
        }
    };

    template <typename Socket, typename ErrorList>
    struct response<void, Socket, ErrorList> :
        response_base<void, Socket, ErrorList>
    {
        using response_base<void, Socket, ErrorList>::response_base;

        template <typename Callable, typename Arguments>
        auto write(
            const ErrorList& errors,
            Callable&& callable,
            Arguments&& args
        ) const -> ext::task<> {
            // TODO If integral encoding is specialized to take a copy instead
            // of reference, this constant could be removed.
            constexpr status_type success = 0;

            auto task = ext::task<>();

            try {
                co_await std::apply(callable, args);
                task = coder<Socket, status_type>::encode(
                    *this->socket,
                    success
                );
            }
            catch (...) {
                task = this->handle_error(std::current_exception(), errors);
            }

            co_await task;
            co_await this->socket->flush();
        }
    };

    template <typename T, typename Socket, typename ErrorList>
    struct coder<Socket, response<T, Socket, ErrorList>> {
        using type = response<T, Socket, ErrorList>;

        static auto decode(Socket& sock) -> ext::task<type> {
            co_return type(sock);
        }
    };
}
