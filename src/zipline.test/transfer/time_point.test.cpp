#include "../socket.test.h"

#include <zipline/zipline>

#include <gtest/gtest.h>

namespace zipline::test {
    using clock = std::chrono::system_clock;
    using time_point = clock::time_point;
    using transfer = zipline::transfer<socket, time_point>;
}

class TimePointTest : public SocketTestBase {};

TEST_F(TimePointTest, ReadWrite) {
    const auto original = zipline::test::clock::now();

    zipline::test::transfer::write(sock, original);

    const auto copy = zipline::test::transfer::read(sock);

    ASSERT_EQ(original, copy);
}
