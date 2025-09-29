#pragma once

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <memory>

#include "utils.hpp"

namespace wwlog {

class LogFlush {
public:
    using Ptr = std::shared_ptr<LogFlush>;

    virtual ~LogFlush() {}

    virtual void Flush(const char* data, size_t len) = 0;
};

class StdoutFlush : public LogFlush {
public:
    using Ptr = std::shared_ptr<StdoutFlush>;
    void Flush(const char* data, size_t len) override { std::cout.write(data, len); }
};

class FileFlush : public LogFlush {
public:
    using Ptr = std::shared_ptr<FileFlush>;

    FileFlush(const std::string& file_name) : file_name_(file_name)
    {
        Utils::File::CreateDirectory(Utils::File::Path(file_name));
        file_ptr_ = fopen(file_name.c_str(), "ab");
        if (file_ptr_ == nullptr) {
            std::cerr << __FILE__ << ":" << __LINE__ << " open log file failed." << std::endl;
            perror(nullptr);
        }
    }
    ~FileFlush()
    {
        if (file_ptr_ != nullptr) {
            fclose(file_ptr_);
            file_ptr_ = nullptr;
        }
    }

    void Flush(const char* data, size_t len) override
    {
        fwrite(data, 1, len, file_ptr_);
        if (ferror(file_ptr_)) {
            std::cerr << __FILE__ << ":" << __LINE__ << " write log file failed." << std::endl;
            perror(nullptr);
        }
        if (wwlog::Utils::LoggerConfig::Instance().flush_log == 1) {
            if (fflush(file_ptr_) == EOF) {
                std::cerr << __FILE__ << ":" << __LINE__ << " fflush file failed." << std::endl;
                perror(nullptr);
            }
        } else if (wwlog::Utils::LoggerConfig::Instance().flush_log == 2) {
            fflush(file_ptr_);
            fsync(fileno(file_ptr_));
        }
    }

private:
    std::string file_name_;
    FILE* file_ptr_ = nullptr;
};

class RollFileFlush : public LogFlush {
public:
    using Ptr = std::shared_ptr<RollFileFlush>;

    RollFileFlush(const std::string& file_name, size_t max_size) : max_size_(max_size), base_name_(file_name)
    {
        Utils::File::CreateDirectory(Utils::File::Path(file_name));
    }
    ~RollFileFlush()
    {
        if (file_ptr_ != nullptr) {
            fclose(file_ptr_);
            file_ptr_ = nullptr;
        }
    }

    void Flush(const char* data, size_t len) override
    {
        InitLogFile();
        fwrite(data, 1, len, file_ptr_);
        if (ferror(file_ptr_)) {
            std::cerr << __FILE__ << ":" << __LINE__ << " write log file failed." << std::endl;
            perror(nullptr);
        }
        current_size_ += len;
        if (wwlog::Utils::LoggerConfig::Instance().flush_log == 1) {
            if (fflush(file_ptr_) == EOF) {
                std::cerr << __FILE__ << ":" << __LINE__ << " fflush file failed." << std::endl;
                perror(nullptr);
            }
        } else if (wwlog::Utils::LoggerConfig::Instance().flush_log == 2) {
            fflush(file_ptr_);
            fsync(fileno(file_ptr_));
        }
    }

private:
    void InitLogFile()
    {
        if (file_ptr_ == nullptr || current_size_ >= max_size_) {
            if (file_ptr_ != nullptr) {
                fclose(file_ptr_);
                file_ptr_ = nullptr;
            }

            std::string file_name = CreateRollFile();
            file_ptr_ = fopen(file_name.c_str(), "ab");
            if (file_ptr_ == nullptr) {
                std::cerr << __FILE__ << ":" << __LINE__ << " open roll log file failed." << std::endl;
                perror(nullptr);
            }
            current_size_ = 0;
        }
    }

    std::string CreateRollFile()
    {
        time_t time = Utils::Date::Now();
        struct tm t;
        localtime_r(&time, &t);
        std::stringstream file_name_stream;
        file_name_stream << base_name_ << std::put_time(&t, "%Y%m%d%H%M") << "-" << file_count++ << ".log";
        return file_name_stream.str();
    }

private:
    size_t file_count = 1;
    size_t current_size_ = 0;
    size_t max_size_;
    std::string base_name_;
    FILE* file_ptr_ = nullptr;
};

class LogFlushFactory {
public:
    using Ptr = std::shared_ptr<LogFlushFactory>;

    template <typename FlushType, typename... Args>
    static std::shared_ptr<LogFlush> CreateLogFlush(Args&&... args)
    {
        return std::make_shared<FlushType>(std::forward<Args>(args)...);
    }
};
}  // namespace wwlog