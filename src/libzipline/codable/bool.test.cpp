#include "coder.test.hpp"

TEST_F(CoderTest, Bool) {
    test(true);
    test(false);
}

TEST_F(CoderTest, BoolError) {
    test_decode_failure<std::uint8_t, bool, std::system_error>(2);
}
