#pragma once

// #include <memory>
#include <thread>

#include "level.hpp"
#include "utils.hpp"

namespace wwlog {
struct LogMessage {
    LogMessage() = default;
    LogMessage(std::string name, LogLevel::Value level, std::string file, size_t line, std::string payload)
        : loggger_name_(name),
          level_(level),
          file_name_(file),
          line_(line),
          payload_(payload),
          time_(Utils::Date::Now()),
          thread_id_(std::this_thread::get_id())
    {
    }

    std::string Format()
    {
        std::stringstream log_message;
        struct tm t;
        localtime_r(&time_, &t);
        char time_buf[64];
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &t);
        log_message << '[' << time_buf << "][" << thread_id_ << "][" << LogLevel::ToString(level_) << "]["
                    << loggger_name_ << "][" << file_name_ << ":" << line_ << "]\t" << payload_ << std::endl;
        return log_message.str();
    }

    size_t line_;
    time_t time_;
    std::string file_name_;
    std::string loggger_name_;
    std::string payload_;
    std::thread::id thread_id_;
    LogLevel::Value level_;
};
}  // namespace wwlog