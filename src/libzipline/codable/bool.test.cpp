#include "coder.test.hpp"

using BoolTest = CoderTest;

TEST_F(BoolTest, True) { test(true); }

TEST_F(BoolTest, False) { test(false); }

TEST_F(BoolTest, Error) {
    test_decode_failure<std::uint8_t, bool, std::system_error>(2);
}
