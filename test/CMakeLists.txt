# CMakeLists.txt for LogSystem tests
cmake_minimum_required(VERSION 3.10)
project(LogSystemTest)

set(CMAKE_CXX_STANDARD 17)

# 添加 gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加 LogSystem 源码目录
include_directories(../)

# 测试源文件
add_executable(logsystem_test
    test_async_buffer.cpp
    test_async_worker.cpp
    test_async_logger.cpp
    test_flush.cpp
    test_thread_pool.cpp
    test_main.cpp
)

target_link_libraries(logsystem_test ${GTEST_LIBRARIES} pthread jsoncpp)
