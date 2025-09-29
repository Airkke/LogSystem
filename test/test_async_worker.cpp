#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include "../async_worker.hpp"

using namespace wwlog;

TEST(AsyncWorkerTest, BasicPushAndCallback)
{
    std::atomic<bool> called{false};
    AsyncWorker worker([&called](Buffer& b) {
        if (b.ReadableSize() > 0) called = true;
    });
    
    worker.Push("test", 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    worker.Stop();
    ASSERT_TRUE(called.load());
}

TEST(AsyncWorkerTest, MultiplePush)
{
    std::atomic<int> count{0};
    AsyncWorker worker([&count](Buffer& b) { count += b.ReadableSize(); });

    worker.Push("abc", 3);
    worker.Push("defg", 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    worker.Stop();
    ASSERT_EQ(count.load(), 7);
}