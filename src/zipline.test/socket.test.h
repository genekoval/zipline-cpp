#pragma once

#include <zipline/zipline>

#include <gtest/gtest.h>

namespace zipline::test {
    using buffer_type = zipline::io::array_buffer<1024>;
}

class SocketTestBase : public testing::Test {
protected:
    zipline::test::buffer_type buffer;
};
