#pragma once

#include "../socket.test.h"

class CoderTest : public SocketTestBase {
protected:
    template <typename T>
    requires
        zipline::codable<T, decltype(buffer)> &&
        std::equality_comparable<T>
    auto test(const T& original) -> void {
        [&]() -> ext::detached_task {
            co_await zipline::encode(original, buffer);
            const auto copy = co_await zipline::decode<T>(buffer);

            EXPECT_EQ(original, copy);

            // Ensure the buffer does not have any excess data left over.
            EXPECT_TRUE(buffer.empty());
        }();
    }
};
