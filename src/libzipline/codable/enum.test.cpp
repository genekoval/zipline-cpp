#include "coder.test.hpp"

namespace {
    enum class data : std::uint32_t { foo, bar, baz };
}

TEST_F(CoderTest, Enum) {
    test(data::foo);
    test(data::bar);
    test(data::baz);
}
