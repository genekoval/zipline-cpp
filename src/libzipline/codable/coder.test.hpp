#pragma once

#include "../socket.test.hpp"

class CoderTest : public SocketTestBase {
protected:
    template <typename T>
    requires zipline::codable<T, decltype(buffer)> &&
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

    template <typename Encode, typename Decode, typename Exception>
    requires zipline::encodable<Encode, decltype(buffer)> &&
             zipline::decodable<Decode, decltype(buffer)>
    auto test_decode_failure(const Encode& value) -> void {
        [&]() -> ext::detached_task {
            co_await zipline::encode(value, buffer);

            EXPECT_THROW(co_await zipline::decode<Decode>(buffer), Exception);
        }();
    }

    template <typename T, typename Exception>
    requires zipline::encodable<T, decltype(buffer)>
    auto test_encode_failure(const T& value) -> void {
        [&]() -> ext::detached_task {
            ;
            EXPECT_THROW(co_await zipline::encode(value, buffer), Exception);
        }();
    }
};
