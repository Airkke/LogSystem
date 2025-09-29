#include <gtest/gtest.h>
#include "../async_logger.hpp"
#include "../flush.hpp"
#include "../level.hpp"
#include "../thread_pool.hpp"
#include <fstream>
#include <cstdio>

using namespace wwlog;

TEST(AsyncLoggerTest, LogBasic) {
    // 初始化线程池
    auto thread_pool = std::make_shared<ThreadPool>(wwlog::Utils::LoggerConfig::Instance().thread_count);
    // 创建一个文件日志输出
    std::string filename = "./logfile/test_async_logger.log";
    std::remove(filename.c_str());

    std::vector<LogFlush::Ptr> flushes;
    flushes.push_back(std::make_shared<FileFlush>(filename));
    AsyncLogger logger("test_logger", flushes, thread_pool, AsyncType::ASYNC_SAFE);

    logger.Info(__FILE__, __LINE__, "Hello %s %d", "world", 123);
    logger.Error(__FILE__, __LINE__, "Error: %s", "fail");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 检查文件内容
    std::ifstream fin(filename, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    fin.close();

    ASSERT_TRUE(content.find("Hello world 123") != std::string::npos);
    ASSERT_TRUE(content.find("Error: fail") != std::string::npos);

    std::remove(filename.c_str());
}
