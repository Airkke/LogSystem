#include <chrono>
#include <iostream>
#include <vector>

#include "../async_logger.hpp"
#include "../flush.hpp"
#include "../thread_pool.hpp"

using namespace wwlog;

int main()
{
    auto logger_config = &Utils::LoggerConfig::Instance("/home/airkke/airkke/LogSystem/config.conf");
    auto thread_pool = std::make_shared<ThreadPool>(logger_config->thread_count);

    std::string filename = "./logfile/bench_async_logger.log";
    std::remove(filename.c_str());

    std::vector<LogFlush::Ptr> flushes;
    flushes.push_back(std::make_shared<FileFlush>(filename));
    AsyncLogger logger("bench_logger", flushes, thread_pool, AsyncType::ASYNC_SAFE);

    const int N = 1000000;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        logger.Info(__FILE__, __LINE__, "Bench log line %d", i);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Logged " << N << " lines in " << elapsed.count() << " seconds." << std::endl;

    std::remove(filename.c_str());
    return 0;
}