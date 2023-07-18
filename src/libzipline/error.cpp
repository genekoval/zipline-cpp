#include <zipline/error.hpp>

namespace zipline {
    auto internal_error::what() const noexcept -> const char* {
        return "internal error";
    }

    unknown_code::unknown_code(status_type status) :
        std::runtime_error(fmt::format("unknown error code ({})", status))
    {}

    zipline_error::zipline_error() : std::runtime_error("zipline error") {}

    zipline_error::zipline_error(const std::string& what) :
        std::runtime_error(what)
    {}

    auto zipline_error::encode(
        io::abstract_writer& writer
    ) const -> ext::task<> {
        return zipline::encode(what(), writer);
    }
}
