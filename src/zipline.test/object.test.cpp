#include "socket.test.h"

#include <zipline/zipline>

#include <gtest/gtest.h>

using socket = zipline::test::socket;

struct data {
    std::string text;
    int number;
    bool b;

    auto operator==(const data&) const -> bool = default;
};

using transfer = zipline::transfer<socket, data>;

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
    const auto original = data {
        .text = "Object Test",
        .number = 42,
        .b = true
    };

    transfer::write(sock, original);
    auto copy = transfer::read(sock);

    ASSERT_EQ(original, copy);
}
