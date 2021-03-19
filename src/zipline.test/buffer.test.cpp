#include <zipline/zipline>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::literals;

using testing::Return;
using testing::SetArrayArgument;
using testing::Invoke;

struct MockSocket {
    MOCK_METHOD(std::size_t, read, (void* dest, std::size_t len), (const));
    MOCK_METHOD(std::size_t, write, (
        const void* src,
        std::size_t len
    ), (const));
};

template <std::size_t N>
using buffered_reader = zipline::buffered_reader<MockSocket, N>;

template <std::size_t N>
using buffered_writer = zipline::buffered_writer<MockSocket, N>;

TEST(BufferTest, ReadZero) {
    auto socket = MockSocket();
    EXPECT_CALL(socket, read).Times(0);

    auto reader = buffered_reader<10>(socket);

    auto i = int();
    reader.read(&i, 0);
}

TEST(BufferTest, ReadOne) {
    constexpr auto character = 'c';
    auto read = [character](void* dest, std::size_t len) -> std::size_t {
        std::memcpy(dest, &character, 1);
        return 1;
    };

    auto socket = MockSocket();
    EXPECT_CALL(socket, read).WillOnce(Invoke(read));

    auto reader = buffered_reader<10>(socket);

    auto c = char();
    reader.read(&c, sizeof(c));

    ASSERT_EQ(character, c);
}

TEST(BufferTest, ReadMany) {
    constexpr auto buffer_size = 2;
    constexpr auto expected = "Hello, world"sv;

    auto source = expected;
    auto read = [&source](void* dest, std::size_t len) -> std::size_t {
        std::memcpy(dest, source.data(), len);
        source = source.substr(len);
        return len;
    };

    auto socket = MockSocket();
    EXPECT_CALL(socket, read)
        .Times(expected.size() / buffer_size)
        .WillRepeatedly(Invoke(read));

    auto reader = buffered_reader<buffer_size>(socket);

    char buffer[expected.size()];
    reader.read(&buffer, sizeof(buffer));

    ASSERT_EQ(expected, buffer);
}

TEST(BufferTest, WriteZero) {
    auto socket = MockSocket();
    EXPECT_CALL(socket, write).Times(0);

    auto writer = buffered_writer<10>(socket);

    auto i = 64;
    writer.write(&i, 0);
}

TEST(BufferTest, WriteOne) {
    constexpr auto character = 'c';
    auto receiver = char();
    auto write = [&receiver](const void* src, std::size_t len) -> std::size_t {
        std::memcpy(&receiver, src, len);
        return 1;
    };

    auto socket = MockSocket();
    EXPECT_CALL(socket, write).WillOnce(Invoke(write));

    { // Force destructor/flush to be called.
        auto writer = buffered_writer<10>(socket);
        writer.write(&character, sizeof(character));
    }

    ASSERT_EQ(character, receiver);
}

TEST(BufferTest, WriteMany) {
    constexpr auto buffer_size = 2;
    constexpr auto data = "Hello, world"sv;

    auto receiver = std::array<char, data.size()>();
    auto index = 0;
    auto write = [&](const void* src, std::size_t len) -> std::size_t {
        std::memcpy(&receiver[index], src, len);
        index += len;
        return len;
    };

    auto socket = MockSocket();
    EXPECT_CALL(socket, write)
        .Times(data.size() / buffer_size)
        .WillRepeatedly(Invoke(write));

    {
        auto writer = buffered_writer<buffer_size>(socket);
        writer.write(data.data(), data.size());
    }

    auto result = std::string(receiver.data(), receiver.size());
    ASSERT_EQ(data, result);
}

TEST(BufferTest, ReadWriteObjects) {
    constexpr auto buffer_size = 1024;
    auto numbers = std::vector<int> {
        4, 8, 15, 16, 23, 42
    };

    auto bytes = std::array<std::byte, 2048>();

    auto read_index = 0;
    auto read = [&](void* dest, std::size_t len) -> std::size_t {
        std::memcpy(dest, &bytes[read_index], len);
        read_index += len;
        return len;
    };

    auto write_index = 0;
    auto write = [&](const void* src, std::size_t len) -> std::size_t {
        std::memcpy(&bytes[write_index], src, len);
        write_index += len;
        return len;
    };

    auto socket = MockSocket();

    EXPECT_CALL(socket, read)
        .WillOnce(Invoke(read));
    EXPECT_CALL(socket, write)
        .WillOnce(Invoke(write));

    {
        auto writer = buffered_writer<buffer_size>(socket);
        zipline::transfer<decltype(writer), decltype(numbers)>::write(
            writer, numbers
        );
    }

    auto reader = buffered_reader<buffer_size>(socket);
    auto result =
        zipline::transfer<decltype(reader), decltype(numbers)>::read(reader);

    ASSERT_EQ(numbers.size(), result.size());

    for (auto i = 0ul; i < result.size(); ++i) {
        ASSERT_EQ(numbers[i], result[i]);
    }
}
