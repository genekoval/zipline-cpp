#include "coder.test.hpp"

using PairTest = CoderTest;

TEST_F(PairTest, SameTypes) {
    using type = std::int16_t;
    using pair = std::pair<type, type>;

    test(pair {0, 300});
}

TEST_F(PairTest, DifferentTypes) {
    test(std::pair<std::int64_t, std::string> {500, "Hello, World!"});
}
