#pragma once

#include <zipline/zipline>

#include <gtest/gtest.h>

class SocketTestBase : public testing::Test {
    std::array<std::byte, 1024> buffer;
protected:
    zipline::memory_buffer socket = zipline::memory_buffer(buffer.data());
};
