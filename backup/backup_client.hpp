#pragma once

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <string>
#include <arpa/inet.h>

#include "../utils.hpp"

void StartBackup(const std::string &message)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        std::cerr << __FILE__ << ":" << __LINE__ << "socket error : " << strerror(errno) << std::endl;
        perror(nullptr);
    }

    // 设置服务器地址结构
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(wwlog::Utils::LoggerConfig::Instance().backup_port);
    inet_aton(wwlog::Utils::LoggerConfig::Instance().backup_addr.c_str(), &server_addr.sin_addr);

    int retry_max_times = 5;
    while (-1 == connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        if (retry_max_times > 0) {
            std::cout << "正在尝试重连，重连次数还有: " << retry_max_times-- << std::endl;
            continue;
        }
        std::cerr << __FILE__ << ":" << __LINE__ << "connect error : " << strerror(errno) << std::endl;
        close(client_socket);
        perror(nullptr);
        return ;
    }

    char buffer[1024];
    if (-1 == write(client_socket, message.c_str(), message.size())) {
        std::cerr << __FILE__ << ":" << __LINE__ << "send to server error : " << strerror(errno) << std::endl;
        perror(nullptr);
    }
    close(client_socket);
}