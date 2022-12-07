#include "coder.test.hpp"

using optional_int = std::optional<std::int32_t>;

using OptionalTest = CoderTest;

TEST_F(OptionalTest, Empty) {
    test(optional_int());
}

TEST_F(OptionalTest, ContainsValue) {
    test<optional_int>(50);
}
