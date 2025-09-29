#include <gtest/gtest.h>
#include "../thread_pool.hpp"
#include <atomic>
#include <thread>
#include <vector>

TEST(ThreadPoolTest, BasicTaskExecution) {
    ThreadPool pool(4);
    std::atomic<int> counter{0};

    // 提交10个任务，每个任务让counter加1
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.Enqueue([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }
    for (auto& f : futures) f.get();

    ASSERT_EQ(counter.load(), 10);
}

TEST(ThreadPoolTest, ParallelTasks) {
    ThreadPool pool(2);
    std::atomic<int> sum{0};
    std::vector<std::future<void>> futures;

    // 提交多个任务并行执行
    for (int i = 0; i < 5; ++i) {
        futures.push_back(pool.Enqueue([&sum, i]() {
            sum.fetch_add(i, std::memory_order_relaxed);
        }));
    }
    for (auto& f : futures) f.get();

    ASSERT_EQ(sum.load(), 0 + 1 + 2 + 3 + 4);
}

TEST(ThreadPoolTest, ReturnValue) {
    ThreadPool pool(2);
    auto fut = pool.Enqueue([]() { return 42; });
    ASSERT_EQ(fut.get(), 42);
}

TEST(ThreadPoolTest, ExceptionHandling) {
    ThreadPool pool(2);
    auto fut = pool.Enqueue([]() -> int { throw std::runtime_error("error"); });
    ASSERT_THROW(fut.get(), std::runtime_error);
}