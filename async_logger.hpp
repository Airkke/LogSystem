#pragma once

#include <atomic>
#include <cassert>
#include <cstdarg>
#include <memory>
#include <mutex>

#include "async_worker.hpp"
#include "backup/backup_client.hpp"
#include "flush.hpp"
#include "level.hpp"
#include "message.hpp"
#include "thread_pool.hpp"

namespace wwlog {
class AsyncLogger {
public:
    using Ptr = std::shared_ptr<AsyncLogger>;

    AsyncLogger(const std::string& logger_name, std::vector<LogFlush::Ptr>& flushes,
                std::shared_ptr<ThreadPool> thread_pool, AsyncType type)
        : logger_name_(logger_name),
          flushes_(flushes),
          thread_pool_(thread_pool),
          async_worker_(
              std::make_shared<AsyncWorker>(std::bind(&AsyncLogger::RealFlush, this, std::placeholders::_1), type))
    {
    }
    virtual ~AsyncLogger() {};

    std::string Name() { return logger_name_; }
    void Trace(const std::string& file, size_t line, const std::string format, ...)
    {
        va_list va;
        va_start(va, format);
        char* ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1) perror("vasprintf failed!");
        va_end(va);

        SerializeAndDispatch(LogLevel::Value::kTrace, file, line, ret);
        free(ret);
        ret = nullptr;
    }
    void Debug(const std::string& file, size_t line, const std::string format, ...)
    {
        va_list va;
        va_start(va, format);
        char* ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1) perror("vasprintf failed!");
        va_end(va);

        SerializeAndDispatch(LogLevel::Value::kDebug, file, line, ret);
        free(ret);
        ret = nullptr;
    }
    void Info(const std::string& file, size_t line, const std::string format, ...)
    {
        va_list va;
        va_start(va, format);
        char* ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1) perror("vasprintf failed!");
        va_end(va);

        SerializeAndDispatch(LogLevel::Value::kInfo, file, line, ret);
        free(ret);
        ret = nullptr;
    }
    void Warn(const std::string& file, size_t line, const std::string format, ...)
    {
        va_list va;
        va_start(va, format);
        char* ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1) perror("vasprintf failed!");
        va_end(va);

        SerializeAndDispatch(LogLevel::Value::kWarn, file, line, ret);
        free(ret);
        ret = nullptr;
    }
    void Error(const std::string& file, size_t line, const std::string format, ...)
    {
        va_list va;
        va_start(va, format);
        char* ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1) perror("vasprintf failed!");
        va_end(va);

        SerializeAndDispatch(LogLevel::Value::kError, file, line, ret);
        free(ret);
        ret = nullptr;
    }
    void Fatal(const std::string& file, size_t line, const std::string format, ...)
    {
        va_list va;
        va_start(va, format);
        char* ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1) perror("vasprintf failed!");
        va_end(va);

        SerializeAndDispatch(LogLevel::Value::kFatal, file, line, ret);
        free(ret);
        ret = nullptr;
    }

protected:
    void SerializeAndDispatch(LogLevel::Value level, const std::string& file, size_t line, char* ret)
    {
        LogMessage message(logger_name_, level, file, line, ret);
        std::string data = message.Format();
        if (level == LogLevel::Value::kFatal || level == LogLevel::Value::kError) {
            try {
                auto ret = thread_pool_->Enqueue(StartBackup, data);
                ret.get();
            } catch (const std::runtime_error& e) {
                std::cerr << __FILE__ << ":" << __LINE__ << "thread pool closed" << std::endl;
            }
        }
        Flush(data.c_str(), data.size());
    }

    void Flush(const char* data, size_t len) { async_worker_->Push(data, len); }

    void RealFlush(Buffer& buffer)
    {
        if (flushes_.empty()) return;
        for (auto& e : flushes_) {
            e->Flush(buffer.Begin(), buffer.ReadableSize());
        }
    }

protected:
    std::mutex mtx_;
    std::string logger_name_;
    std::vector<LogFlush::Ptr> flushes_;
    std::shared_ptr<ThreadPool> thread_pool_;
    wwlog::AsyncWorker::AsyncWorkerPtr async_worker_;
};

class LoggerBuilder {
public:
    using Ptr = std::shared_ptr<LoggerBuilder>;

    void SetLoggerName(const std::string& name) { logger_name_ = name; }
    void SetLoggerType(AsyncType type) { async_type_ = type; }
    void SetThreadPool(std::shared_ptr<ThreadPool> thread_pool) { thread_pool_ = std::move(thread_pool); }
    template <typename FlushType, typename... Args>
    void AddLoggerFlush(Args&&... args)
    {
        flushes_.emplace_back(LogFlushFactory::CreateLogFlush<FlushType>(std::forward<Args>(args)...));
    }
    AsyncLogger::Ptr Build()
    {
        assert(logger_name_.empty() == false);

        if (flushes_.empty()) flushes_.emplace_back(std::make_shared<StdoutFlush>());
        return std::make_shared<AsyncLogger>(logger_name_, flushes_, thread_pool_, async_type_);
    }

protected:
    std::string logger_name_ = "async_logger";      // 默认日志器名称
    std::vector<wwlog::LogFlush::Ptr> flushes_;     // 日志输出方式
    std::shared_ptr<ThreadPool> thread_pool_;       // 线程池
    AsyncType async_type_ = AsyncType::ASYNC_SAFE;  // 用于控制缓冲区是否增长
};
}  // namespace wwlog