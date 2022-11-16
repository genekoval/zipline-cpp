#include "socket.test.h"

#include <zipline/zipline>

#include <gtest/gtest.h>

namespace {
    struct data {
        std::string text;
        int number;
        bool b;

        auto operator==(const data&) const -> bool = default;
    };

    using coder = zipline::coder<zipline::memory_buffer, data>;
}

namespace zipline {
    ZIPLINE_OBJECT(
        data,
        &data::text,
        &data::number,
        &data::b
    );
}

class ObjectTest : public SocketTestBase {};

TEST_F(ObjectTest, ReadWrite) {
    [this]() -> ext::detached_task {
        const auto original = data {
            .text = "Object Test",
            .number = 42,
            .b = true
        };

        co_await coder::encode(socket, original);
        auto copy = co_await coder::decode(socket);

        EXPECT_EQ(original, copy);
    }();
}
