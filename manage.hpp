#pragma once

#include <unordered_map>

#include "async_logger.hpp"

namespace wwlog {
class LoggerManager {
public:
    static LoggerManager &GetInstance()
    {
        static LoggerManager manager;
        return manager;
    }

    bool LoggerExist(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        return loggers_.find(name) != loggers_.end();
    }

    void AddLogger(const AsyncLogger::Ptr &&async_logger)
    {
        if (LoggerExist(async_logger->Name())) return;
        std::unique_lock<std::mutex> lock(mtx_);
        loggers_.insert(std::make_pair(async_logger->Name(), async_logger));
    }

    AsyncLogger::Ptr GetLogger(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(mtx_);
        auto it = loggers_.find(name);
        if (it == loggers_.end()) return AsyncLogger::Ptr();
        return it->second;
    }

    AsyncLogger::Ptr DefaultLogger() { return default_logger_; }

private:
    LoggerManager()
    {
        std::unique_ptr<LoggerBuilder> builder(new LoggerBuilder());
        builder->SetLoggerName("default");
        default_logger_ = builder->Build();
        loggers_.insert(std::make_pair("default", default_logger_));
    }

private:
    std::mutex mtx_;
    AsyncLogger::Ptr default_logger_;
    std::unordered_map<std::string, AsyncLogger::Ptr> loggers_;
};
}  // namespace wwlog