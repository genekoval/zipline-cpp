#include "coder.test.hpp"

TEST_F(CoderTest, TimePoint) {
    test(std::chrono::system_clock::now());
}
