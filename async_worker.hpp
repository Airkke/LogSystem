#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include "async_buffer.hpp"

namespace wwlog {
enum class AsyncType { ASYNC_SAFE, ASYNC_UNSAFE };
using CallBackFunctor = std::function<void(Buffer&)>;

class AsyncWorker {
public:
    using AsyncWorkerPtr = std::shared_ptr<AsyncWorker>;

    AsyncWorker(const CallBackFunctor& callback, AsyncType async_type = AsyncType::ASYNC_SAFE)
        : async_type_(async_type),
          callback_(callback),
          stop_(false),
          thread_(std::thread(&AsyncWorker::ThreadEntry, this))
    {
    }
    ~AsyncWorker() { Stop(); }

    void Push(const char* data, size_t len)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        // 安全模式下，不允许缓冲区增长
        if (AsyncType::ASYNC_SAFE == async_type_) {
            cond_productor_.wait(lock, [&]() { return len <= buffer_productor_.WriteableSize(); });
        }
        buffer_productor_.Push(data, len);
        cond_consumer_.notify_one();
    }
    void Stop()
    {
        stop_ = true;
        cond_consumer_.notify_all();
        if (thread_.joinable()) thread_.join();
    }

private:
    void ThreadEntry()
    {
        while (true) {
            {
                std::unique_lock<std::mutex> lock(mtx_);
                if (buffer_productor_.IsEmpty() && stop_) {
                    cond_consumer_.wait(lock, [&]() { return stop_ || !buffer_productor_.IsEmpty(); });
                }
                buffer_productor_.Swap(buffer_consumer_);
                // 安全模式下，通知生产者检查是否满足写入条件
                if (async_type_ == AsyncType::ASYNC_SAFE) cond_productor_.notify_one();
            }
            callback_(buffer_consumer_);
            buffer_consumer_.Reset();
            if (stop_ && buffer_productor_.IsEmpty()) return;
        }
    }

private:
    AsyncType async_type_;
    std::atomic<bool> stop_;
    std::mutex mtx_;
    wwlog::Buffer buffer_productor_;
    wwlog::Buffer buffer_consumer_;
    std::condition_variable cond_productor_;
    std::condition_variable cond_consumer_;
    std::thread thread_;

    CallBackFunctor callback_;
};
}  // namespace wwlog