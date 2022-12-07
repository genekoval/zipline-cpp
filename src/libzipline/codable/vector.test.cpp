#include "coder.test.hpp"

using int_vector = std::vector<std::int32_t>;

using VectorTest = CoderTest;

TEST_F(VectorTest, Empty) {
    test<int_vector>({});
}

TEST_F(VectorTest, OneValue) {
    test<int_vector>({0});
}

TEST_F(VectorTest, MultipleValues) {
    test<int_vector>({1, 2});
}
