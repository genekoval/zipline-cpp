#pragma once

#include "error_list.hpp"

namespace zipline::detail {
    template <io::writer Writer>
    class responder {
        const error_codes& errors;
    protected:
        Writer& writer;

        auto handle_error(std::exception_ptr ex) const -> ext::task<> {
            status_type code = 1;
            auto wrapper = io::abstract_writer_wrapper(writer);
            auto task = std::optional<ext::task<>>();

            try {
                std::rethrow_exception(ex);
            }
            catch (const zipline_error& error) {
                const auto result = errors.code(error);

                if (result) {
                    code = *result + 2;
                    task = error.encode(wrapper);
                }
                else { TIMBER_ERROR(error.what()); }
            }
            catch (const std::exception& ex) {
                TIMBER_ERROR(ex.what());
            }
            catch (...) {
                TIMBER_ERROR("Unknown error occurred during request");
            }

            co_await zipline::encode(code, writer);
            if (task) co_await *task;
        }

        responder(const error_codes& errors, Writer& writer) :
            errors(errors),
            writer(writer) {}
    };
}

namespace zipline {
    template <typename T, io::reader Reader>
    requires decodable<T, Reader> || std::is_void_v<T>
    class response {
        const error_thrower& errors;
        Reader& reader;
    public:
        response(const error_thrower& errors, Reader& reader) :
            errors(errors),
            reader(reader) {}

        auto read() const -> ext::task<T> {
            const auto status = co_await zipline::decode<status_type>(reader);

            if (status == 0) {
                if constexpr (std::is_void_v<T>) co_return;
                else co_return co_await zipline::decode<T>(reader);
            }

            if (status == 1) throw internal_error();

            auto wrapper = io::abstract_reader_wrapper(reader);
            co_await errors.throw_error(status - 2, wrapper);
            throw unknown_code(status);
        }
    };

    template <typename T, io::writer Writer>
    requires encodable<T, Writer> || std::is_void_v<T>
    struct responder : private detail::responder<Writer> {
        responder(const error_codes& errors, Writer& writer) :
            detail::responder<Writer>(errors, writer) {}

        template <typename Callable, typename Args>
        auto write(Callable&& callable, Args&& args) const -> ext::task<> {
            auto result = std::optional<T>();
            auto task = ext::task<>();

            try {
                result = co_await std::apply(
                    std::forward<Callable>(callable),
                    std::forward<Args>(args)
                );
                task = [](const T& result, Writer& writer) -> ext::task<> {
                    co_await zipline::encode<status_type>(0, writer);
                    co_await zipline::encode(result, writer);
                }(*result, this->writer);
            }
            catch (...) {
                task = this->handle_error(std::current_exception());
            }

            co_await task;
            co_await this->writer.flush();
        }
    };

    template <io::writer Writer>
    struct responder<void, Writer> : private detail::responder<Writer> {
        responder(const error_codes& errors, Writer& writer) :
            detail::responder<Writer>(errors, writer) {}

        template <typename Callable, typename Args>
        auto write(Callable&& callable, Args&& args) const -> ext::task<> {
            auto task = ext::task<>();

            try {
                co_await std::apply(callable, args);
                task = zipline::encode<status_type>(0, this->writer);
            }
            catch (...) {
                task = this->handle_error(std::current_exception());
            }

            co_await task;
            co_await this->writer.flush();
        }
    };

    template <typename T, io::reader Reader>
    requires(decodable<T, Reader> || std::is_void_v<T>) &&
            requires(const Reader& reader) {
                { reader.errors } -> std::same_as<const error_thrower&>;
            }
    struct decoder<response<T, Reader>, Reader> {
        static auto decode(Reader& reader) -> ext::task<response<T, Reader>> {
            co_return response<T, Reader>(reader.errors, reader);
        }
    };
}
