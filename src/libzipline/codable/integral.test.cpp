#include "coder.test.hpp"

#include <limits>

using std::numeric_limits;

class IntegralTest : public CoderTest {
protected:
    template <std::integral T>
    requires zipline::codable<T, decltype(buffer)>
    auto test_integral() -> void {
        test(numeric_limits<T>::min());
        test(numeric_limits<T>::max());
        test<T>(0);
        test<T>(1);
        test<T>(2);
        test<T>(100);
    }
};

TEST_F(IntegralTest, Int8) {
    test_integral<std::int8_t>();
}

TEST_F(IntegralTest, Int16) {
    test_integral<std::int16_t>();
}

TEST_F(IntegralTest, Int32) {
    test_integral<std::int32_t>();
}

TEST_F(IntegralTest, Int64) {
    test_integral<std::int64_t>();
}

TEST_F(IntegralTest, UInt8) {
    test_integral<std::uint8_t>();
}

TEST_F(IntegralTest, UInt16) {
    test_integral<std::uint16_t>();
}

TEST_F(IntegralTest, UInt32) {
    test_integral<std::uint32_t>();
}

TEST_F(IntegralTest, UInt64) {
    test_integral<std::uint64_t>();
}
