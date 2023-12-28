#include "coder.test.hpp"

namespace {
    struct data {
        std::string text;
        std::int32_t number;
        bool b;

        auto operator==(const data&) const -> bool = default;
    };
}

namespace zipline {
    ZIPLINE_OBJECT(data, &data::text, &data::number, &data::b);
}

TEST_F(CoderTest, Object) {
    test(data {.text = "Object Test", .number = 42, .b = true});
}
