#include "coder.test.hpp"

using zipline::test::buffer_type;

namespace {
    enum class data : std::uint32_t {
        foo,
        bar,
        baz
    };
}

TEST_F(CoderTest, Enum) {
    test(data::foo);
    test(data::bar);
    test(data::baz);
}
