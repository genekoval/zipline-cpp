#include <zipline/error.hpp>

namespace zipline {
    auto eof::what() const noexcept -> const char* {
        return "unexpected EOF";
    }

    auto insufficient_space::what() const noexcept -> const char* {
        return "insufficient space";
    }

    auto internal_error::what() const noexcept -> const char* {
        return "internal error";
    }

    unknown_code::unknown_code(status_type status) :
        std::runtime_error(fmt::format("unknown error code ({})", status))
    {}

    auto zipline_error::encode(
        io::abstract_writer& writer
    ) const -> ext::task<> {
        return zipline::encode(what(), writer);
    }
}