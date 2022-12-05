#pragma once

#include "error.hpp"
#include "error_list.hpp"
#include "codable/codable"

namespace zipline {
    template <
        typename T,
        typename Socket,
        typename ErrorList
    >
    requires io::reader<Socket> && io::writer<Socket>
    class response_base {
    protected:
        Socket* socket;

        auto handle_error(
            std::exception_ptr ex,
            const ErrorList& errors
        ) const -> ext::task<> {
            status_type code = 0;
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
                task = encode(message, *socket);
            }

            co_await encode(code, *socket);
            co_await task;
        }
    public:
        response_base(Socket& socket) : socket(&socket) {}

        auto read(const ErrorList& errors) const -> ext::task<T> {
            const auto status = co_await decode<status_type>(*socket);

            switch (status) {
                case 0:
                    if constexpr (std::is_void_v<T>) co_return;
                    else co_return co_await decode<T>(*socket);
                case 1: {
                    const auto message = co_await decode<std::string>(*socket);
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
                    co_await encode<status_type>(0, socket);
                    co_await encode(result, socket);
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
            auto task = ext::task<>();

            try {
                co_await std::apply(callable, args);
                task = encode<status_type>(0, *this->socket);
            }
            catch (...) {
                task = this->handle_error(std::current_exception(), errors);
            }

            co_await task;
            co_await this->socket->flush();
        }
    };

    template <typename T, io::reader Reader, typename ErrorList>
    struct decoder<response<T, Reader, ErrorList>, Reader> {
        using type = response<T, Reader, ErrorList>;

        static auto decode(Reader& sock) -> ext::task<type> {
            co_return type(sock);
        }
    };
}
