#include <gtest/gtest.h>
#include "../async_buffer.hpp"

using namespace wwlog;

TEST(AsyncBufferTest, BasicPushAndRead) {
    Buffer buf;
    const char* data = "abcdef";
    buf.Push(data, 6);
    ASSERT_EQ(buf.ReadableSize(), 6);
    ASSERT_EQ(std::string(buf.Begin(), 6), "abcdef");
}

TEST(AsyncBufferTest, MoveReadPos) {
    Buffer buf;
    buf.Push("123456", 6);
    buf.MoveReadPos(3);
    ASSERT_EQ(buf.ReadableSize(), 3);
    ASSERT_EQ(std::string(buf.Begin(), 3), "456");
}

TEST(AsyncBufferTest, SwapBuffers) {
    Buffer buf1, buf2;
    buf1.Push("xyz", 3);
    buf1.Swap(buf2);
    ASSERT_EQ(buf1.ReadableSize(), 0);
    ASSERT_EQ(buf2.ReadableSize(), 3);
    ASSERT_EQ(std::string(buf2.Begin(), 3), "xyz");
}

TEST(AsyncBufferTest, ResetBuffer) {
    Buffer buf;
    buf.Push("test", 4);
    buf.Reset();
    ASSERT_EQ(buf.ReadableSize(), 0);
    ASSERT_TRUE(buf.IsEmpty());
}

TEST(AsyncBufferTest, ToBeEnoughAutoExpand) {
    Buffer buf;
    size_t old_size = buf.WriteableSize();
    std::string bigdata(old_size + 10, 'a');
    buf.Push(bigdata.data(), bigdata.size());
    ASSERT_GE(buf.ReadableSize(), old_size + 10);
    ASSERT_EQ(std::string(buf.Begin(), bigdata.size()), bigdata);
}
