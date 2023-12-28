#include <zipline/error_list.hpp>

namespace zipline {
    error_codes::error_codes(map_type&& codes) :
        codes(std::forward<map_type>(codes)) {}

    auto error_codes::code(const std::exception& ex) const
        -> std::optional<status_type> {
        const auto result = codes.find(std::type_index(typeid(ex)));

        if (result == codes.end()) return {};
        return result->second;
    }

    error_thrower::error_thrower(thrower_list&& throwers, std::size_t size) :
        throwers(std::forward<thrower_list>(throwers)),
        size(size) {}

    auto error_thrower::throw_error(
        status_type status,
        io::abstract_reader& reader
    ) const -> ext::task<> {
        if (status >= size) co_return;
        co_await throwers[status](reader);
    }
}
