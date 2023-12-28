#pragma once

#include "../codable.hpp"

#define ZIPLINE_OBJECT_DECODER_TEMPLATE(Type, ...)                             \
    struct decoder<Type, Reader> {                                             \
        static auto decode(Reader& reader) -> ext::task<Type> {                \
            co_return co_await zipline::object::decoder<Type, Reader>::decode( \
                reader,                                                        \
                __VA_ARGS__                                                    \
            );                                                                 \
        }                                                                      \
    };

#define ZIPLINE_OBJECT_DECODER(Type, ...)                                      \
    template <zipline::io::reader Reader>                                      \
    ZIPLINE_OBJECT_DECODER_TEMPLATE(Type, __VA_ARGS__)

#define ZIPLINE_OBJECT_ENCODER_TEMPLATE(Type, ...)                             \
    struct encoder<Type, Writer> {                                             \
        static auto encode(const Type& t, Writer& writer) -> ext::task<> {     \
            co_await zipline::object::encoder<Type, Writer>::encode(           \
                t,                                                             \
                writer,                                                        \
                __VA_ARGS__                                                    \
            );                                                                 \
        }                                                                      \
    };

#define ZIPLINE_OBJECT_ENCODER(Type, ...)                                      \
    template <zipline::io::writer Writer>                                      \
    ZIPLINE_OBJECT_ENCODER_TEMPLATE(Type, __VA_ARGS__)

#define ZIPLINE_OBJECT(Type, ...)                                              \
    ZIPLINE_OBJECT_DECODER(Type, __VA_ARGS__)                                  \
    ZIPLINE_OBJECT_ENCODER(Type, __VA_ARGS__)

namespace zipline::object {
    template <std::default_initializable T, io::reader Reader>
    class decoder {
        template <typename Type, typename Base>
        static auto decode_member(T& t, Reader& reader, Type Base::*member)
            -> ext::task<> {
            t.*member = co_await zipline::decode<Type>(reader);
        }
    public:
        template <typename... Members>
        static auto decode(Reader& reader, Members... members) -> ext::task<T> {
            auto result = T();

            (co_await decode_member(result, reader, members), ...);

            co_return result;
        }
    };

    template <typename T, io::writer Writer>
    class encoder {
        template <typename Type, typename Base>
        static auto encode_member(
            const T& t,
            Writer& writer,
            Type Base::*member
        ) -> ext::task<> {
            co_await zipline::encode(t.*member, writer);
        }
    public:
        template <typename... Members>
        static auto encode(const T& t, Writer& writer, Members... members)
            -> ext::task<> {
            (co_await encode_member(t, writer, members), ...);
        }
    };
}
