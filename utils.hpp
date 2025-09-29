#pragma once

#include <jsoncpp/json/json.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <atomic>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace wwlog {
namespace Utils {

class Date {
public:
    static time_t Now() { return time(nullptr); }
};

class File {
public:
    static bool Exists(const std::string& file_name)
    {
        struct stat st;
        return (0 == stat(file_name.c_str(), &st));
    }

    // 获取文件路径
    static std::string Path(const std::string& file_name)
    {
        if (file_name.empty()) return "";

        size_t pos = file_name.find_last_of("/\\");
        if (pos != std::string::npos) return file_name.substr(0, pos + 1);

        return "";
    }

    static void CreateDirectory(const std::string& path_name)
    {
        if (path_name.empty()) perror("文件所给路径为空：");
        if (!Exists(path_name)) {
            std::error_code error_code;
            std::filesystem::create_directories(std::filesystem::path(path_name), error_code);
            if (error_code) {
                std::cerr << "CreateDirectory failed. " << error_code.message() << std::endl;
            }
        }
    }

    size_t Size(std::string file_name)
    {
        struct stat file_stat;
        int ret = stat(file_name.c_str(), &file_stat);
        if (ret == -1) {
            perror("Get file size failed.\n");
            return -1;
        }
        return file_stat.st_size;
    }

    bool GetContent(std::string* content, std::string file_name)
    {
        // 打开文件
        std::ifstream ifs;
        ifs.open(file_name, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "file open error" << std::endl;
            return false;
        }

        // 读入内容到 content
        ifs.seekg(std::ios::beg);
        size_t file_size = Size(file_name);
        content->resize(file_size);
        ifs.read(content->data(), file_size);
        if (!ifs.good()) {
            std::cout << __FILE__ << ":" << __LINE__ << " readfile content error" << std::endl;
            ifs.close();
            return false;
        }
        ifs.close();
        return true;
    }
};

class JsonConverter {
public:
    static bool ToString(const Json::Value& input, std::string* output)
    {
        Json::StreamWriterBuilder write_builder;
        std::unique_ptr<Json::StreamWriter> writer(write_builder.newStreamWriter());
        std::stringstream json_stream;
        if (writer->write(input, &json_stream) != 0) {
            std::cerr << "Serialize error." << std::endl;
            return false;
        }
        *output = json_stream.str();
        return true;
    }

    static bool FromJsonString(const std::string& input, Json::Value* output)
    {
        Json::CharReaderBuilder reader_builder;
        std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
        std::string err;
        if (reader->parse(input.c_str(), input.c_str() + input.size(), output, &err) == false) {
            std::cerr << __FILE__ << ":" << __LINE__ << " parse error." << err << std::endl;
            return false;
        }
        return true;
    }
};

struct LoggerConfig {
    static LoggerConfig& Instance(const std::string& config_path = "")
    {
        static LoggerConfig logger_config;
        if (!config_path.empty() && !logger_config.initialized_) {
            logger_config.LoadConfig(config_path);
        }
        return logger_config;
    }

private:
    LoggerConfig() : initialized_(false) {}
    void LoadConfig(const std::string& config_path)
    {
        std::string content;
        wwlog::Utils::File file;
        if (file.GetContent(&content, config_path) == false) {
            std::cerr << __FILE__ << ":" << __LINE__ << " open config file failed." << std::endl;
            perror(NULL);
            return;
        }
        Json::Value root;
        wwlog::Utils::JsonConverter::FromJsonString(content, &root);
        buffer_size = root["buffer_size"].asInt64();
        threshold = root["threshold"].asInt64();
        linear_growth = root["linear_growth"].asInt64();
        flush_log = root["flush_log"].asInt64();
        backup_addr = root["backup_addr"].asString();
        backup_port = root["backup_port"].asInt();
        thread_count = root["thread_count"].asInt64();

        initialized_ = true;
    }

public:
    size_t buffer_size;    // 缓冲区基础容量
    size_t threshold;      // 倍数扩容阈值
    size_t linear_growth;  // 线性增长容量
    size_t flush_log;
    std::string backup_addr;
    uint16_t backup_port;
    size_t thread_count;

private:
    std::atomic<bool> initialized_;
};

}  // namespace Utils
}  // namespace wwlog