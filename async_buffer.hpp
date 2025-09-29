#pragma once

#include <cassert>
#include <string>
#include <vector>

#include "utils.hpp"

namespace wwlog {

// 注意：本类所有公有方法都不是线程安全的，使用时需由调用方加锁保护！
class Buffer {
public:
    Buffer() : write_pos_(0), read_pos_(0) { buffer_.resize(wwlog::Utils::LoggerConfig::Instance().buffer_size); }

    // 写入相关
    void Push(const char* data, size_t len)
    {
        ToBeEnough(len);
        std::copy(data, data + len, &buffer_[write_pos_]);
        write_pos_ += len;
    }
    size_t WriteableSize() { return buffer_.size() - write_pos_; }

    // 读取相关
    const char* Begin() { return &buffer_[read_pos_]; }
    char* ReadBegin(int len)
    {
        assert(len <= ReadableSize());
        return &buffer_[read_pos_];
    }
    size_t ReadableSize() { return write_pos_ - read_pos_; }
    void MoveReadPos(int len)
    {
        assert(len <= ReadableSize());
        read_pos_ += len;
    }

    // 其他操作
    bool IsEmpty() { return write_pos_ == read_pos_; }
    void Swap(Buffer& buf)
    {
        buffer_.swap(buf.buffer_);
        std::swap(read_pos_, buf.read_pos_);
        std::swap(write_pos_, buf.write_pos_);
    }
    void Reset()
    {
        write_pos_ = 0;
        read_pos_ = 0;
    }

protected:
    void ToBeEnough(size_t len)
    {
        int buffer_size = buffer_.size();
        if (len >= WriteableSize()) {
            if (buffer_.size() < wwlog::Utils::LoggerConfig::Instance().threshold) {
                buffer_.resize(2 * buffer_.size() + buffer_size);
            } else {
                buffer_.resize(wwlog::Utils::LoggerConfig::Instance().linear_growth + buffer_size);
            }
        }
    }

protected:
    std::vector<char> buffer_;
    size_t write_pos_;
    size_t read_pos_;
};

}  // namespace wwlog
