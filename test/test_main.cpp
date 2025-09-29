#include <gtest/gtest.h>

#include "../utils.hpp"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // 初始化全局配置对象
    auto logger_config = &wwlog::Utils::LoggerConfig::Instance("/home/airkke/airkke/LogSystem/config.conf");

    // 输出全局配置

    std::cout << "buffer_size = " << logger_config->buffer_size << std::endl;
    std::cout << "threshold = " << logger_config->threshold << std::endl;
    std::cout << "linear_growth = " << logger_config->linear_growth << std::endl;
    std::cout << "flush_log = " << logger_config->flush_log << std::endl;
    std::cout << "backup_addr = " << logger_config->backup_addr << std::endl;
    std::cout << "backup_port = " << logger_config->backup_port << std::endl;
    std::cout << "thread_count = " << logger_config->thread_count << std::endl;

    return RUN_ALL_TESTS();
}