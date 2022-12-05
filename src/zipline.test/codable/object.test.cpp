#include "../socket.test.h"

using zipline::test::buffer_type;

namespace {
    struct data {
        std::string text;
        std::int32_t number;
        bool b;

        auto operator==(const data&) const -> bool = default;
    };
}

namespace zipline {
    ZIPLINE_OBJECT(
        data,
        &data::text,
        &data::number,
        &data::b
    );
}

using ObjectTest = SocketTestBase;

TEST_F(ObjectTest, ReadWrite) {
    [&]() -> ext::detached_task {
        const auto original = data {
            .text = "Object Test",
            .number = 42,
            .b = true
        };

        co_await zipline::encode(original, buffer);
        auto copy = co_await zipline::decode<data>(buffer);

        EXPECT_EQ(original, copy);
    }();
}
