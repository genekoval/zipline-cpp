#include "../socket.test.h"

using zipline::test::buffer_type;

namespace {
    using clock = std::chrono::system_clock;
    using time_point = clock::time_point;
    using decoder = zipline::decoder<time_point, buffer_type>;
    using encoder = zipline::encoder<time_point, buffer_type>;
}

class TimePointTest : public SocketTestBase {};

TEST_F(TimePointTest, ReadWrite) {
    [&]() -> ext::detached_task {
        const auto original = clock::now();
        co_await encoder::encode(original, buffer);

        const auto copy = co_await decoder::decode(buffer);
        EXPECT_EQ(original, copy);
    }();
}
