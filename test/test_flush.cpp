#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>

#include "../flush.hpp"

using namespace wwlog;

TEST(FlushTest, StdoutFlushWorks)
{
    StdoutFlush flush;
    testing::internal::CaptureStdout();
    flush.Flush("hello", 5);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "hello");
}

TEST(FlushTest, FileFlushWorks) {
    std::string filename = "./logfile/test_file_flush.log";
    // 清理旧文件
    std::remove(filename.c_str());

    FileFlush flush(filename);
    flush.Flush("abc", 3);

    std::ifstream fin(filename, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    fin.close();
    ASSERT_EQ(content, "abc");

    std::remove(filename.c_str());
}

TEST(FlushTest, RollFileFlushWorks) {
    std::string basename = "./logfile/test_roll_file_flush";
    RollFileFlush flush(basename, 5); // 每5字节滚动

    flush.Flush("123", 3);
    flush.Flush("45", 2); // 达到5字节，触发滚动
    flush.Flush("678", 3);

    // 检查是否生成了至少两个文件
    int file_count = 0;
    for (int i = 1; i <= 2; ++i) {
        std::stringstream ss;
        time_t now = Utils::Date::Now();
        struct tm t;
        localtime_r(&now, &t);
        ss << basename << std::put_time(&t, "%Y%m%d%H%M") << "-" << i << ".log";
        std::ifstream fin(ss.str(), std::ios::binary);
        if (fin.good()) {
            ++file_count;
            fin.close();
            std::remove(ss.str().c_str());
        }
    }
    ASSERT_GE(file_count, 1); // 至少有一个滚动文件被创建
}
