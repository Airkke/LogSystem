#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>

using func_t = std::function<void(const std::string &)>;
const int backlog = 32;

class TcpServer;

class ThreadData {
public:
    ThreadData(int fd, const std::string &ip, const uint16_t &port, TcpServer *tcp_server_ptr)
        : sock(fd), client_ip(ip), client_port(port), tcp_server_ptr_(tcp_server_ptr)
    {
    }

public:
    int sock;
    std::string client_ip;
    uint16_t client_port;
    TcpServer *tcp_server_ptr_;
};

class TcpServer {
public:
    TcpServer(uint16_t port, func_t func) : port_(port), func_(func) {}
    ~TcpServer() = default;

    void InitService()
    {
        listen_sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock_ == -1) {
            std::cout << __FILE__ << ":" << __LINE__ << "create socket error" << strerror(errno) << std::endl;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(listen_sock_, (sockaddr *)&server_addr, sizeof(server_addr))) {
            std::cout << __FILE__ << ":" << __LINE__ << "bind socket error" << strerror(errno) << std::endl;
        }

        if (listen(listen_sock_, backlog) < 0) {
            std::cout << __FILE__ << ":" << __LINE__ << "listen error" << strerror(errno) << std::endl;
        }
    }
    static void *ThreadRoutine(void *args)
    {
        pthread_detach(pthread_self());

        ThreadData *thread_data = static_cast<ThreadData *>(args);
        std::string client_info = thread_data->client_ip + ":" + std::to_string(thread_data->client_port);
        thread_data->tcp_server_ptr_->Serive(thread_data->sock, move(client_info));
        close(thread_data->sock);
        delete thread_data;
        return nullptr;
    }
    void StartService()
    {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr_len);
            int client_sock = accept(listen_sock_, (sockaddr *)&client_addr, &client_addr_len);
            if (client_sock < 0) {
                std::cout << __FILE__ << ":" << __LINE__ << "accept error" << strerror(errno) << std::endl;
                continue;
            }

            std::string client_ip = inet_ntoa(client_addr.sin_addr);
            uint16_t client_port = ntohs(client_addr.sin_port);

            pthread_t thread_id;
            ThreadData *thread_data = new ThreadData(client_sock, client_ip, client_port, this);
            pthread_create(&thread_id, nullptr, ThreadRoutine, thread_data);
        }
    }
    void Serive(int sock, const std::string &&client_info)
    {
        char buf[1024];

        int ret = read(sock, buf, sizeof(buf));
        if (ret == -1) {
            std::cout << __FILE__ << ":" << __LINE__ << "read error" << strerror(errno) << std::endl;
            perror(NULL);
        } else if (ret > 0) {
            buf[ret] = 0;
            std::string tmp = buf;
            func_(client_info + tmp);
        }
    }

private:
    int listen_sock_;
    uint16_t port_;
    func_t func_;
};