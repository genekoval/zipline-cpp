#pragma once

#include "../codable.hpp"

namespace zipline {
    template <typename F>
    concept stream_reader = requires(
        const F& f,
        std::span<const std::byte> bytes
    ) {
        { f(bytes) } -> std::same_as<ext::task<>>;
    };

    template <io::reader Reader>
    class stream {
        std::span<const std::byte> chunk;
        std::optional<std::reference_wrapper<Reader>> reader;
        std::optional<std::size_t> stream_size;
    public:
        stream() = default;

        stream(Reader& reader) : reader(reader) {}

        auto read(const stream_reader auto& pipe) -> ext::task<> {
            auto remaining = co_await size();

            co_await pipe(co_await peek());
            remaining -= chunk.size();

            while (remaining > 0) {
                chunk = co_await reader->get().read(remaining);
                remaining -= chunk.size();

                co_await pipe(chunk);
            }
        }

        auto peek() -> ext::task<std::span<const std::byte>> {
            if (!chunk.data()) {
                const auto bytes = co_await size();
                chunk = co_await reader->get().read(bytes);
            }

            co_return chunk;
        }

        auto size() -> ext::task<std::size_t> {
            if (!stream_size) {
                stream_size = co_await zipline::decode<std::size_t>(
                    reader.value().get()
                );
            }

            co_return *stream_size;
        }
    };

    template <io::reader Reader>
    struct decoder<stream<Reader>, Reader> {
        static auto decode(Reader& reader) -> ext::task<stream<Reader>> {
            co_return stream(reader);
        }
    };
}
