#include "../socket.test.h"

#include <zipline/zipline>

namespace {
    namespace internal {
        using clock = std::chrono::system_clock;
        using time_point = clock::time_point;
        using coder = zipline::coder<zipline::memory_buffer, time_point>;
    }
}

class TimePointTest : public SocketTestBase {};

TEST_F(TimePointTest, ReadWrite) {
    [this]() -> ext::detached_task {
        const auto original = internal::clock::now();
        co_await internal::coder::encode(socket, original);

        const auto copy = co_await internal::coder::decode(socket);
        EXPECT_EQ(original, copy);
    }();
}
