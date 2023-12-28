#pragma once

#include "codable/codable"

#include <stdexcept>

namespace zipline {
    using status_type = std::uint8_t;

    struct internal_error : std::exception {
        auto what() const noexcept -> const char* override;
    };

    struct unknown_code : std::runtime_error {
        unknown_code(status_type status);
    };

    struct zipline_error : virtual std::runtime_error {
        zipline_error();

        zipline_error(const std::string& what);

        virtual ~zipline_error() = default;

        virtual auto encode(io::abstract_writer& writer) const -> ext::task<>;
    };

    template <std::derived_from<zipline_error> T>
    struct decoder<T, io::abstract_reader> {
        static auto decode(io::abstract_reader& reader) -> ext::task<T> {
            const auto message = co_await zipline::decode<std::string>(reader);
            throw T {message};
        }
    };

    static_assert(decodable<zipline_error>);
}
