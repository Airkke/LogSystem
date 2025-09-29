#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "backup_server.hpp"

const std::string file_name = "./logfile.log";

void Usage(std::string progress) { std::cout << "Usage error:" << progress << " port" << std::endl; }

bool FileExist(const std::string &name)
{
    struct stat exist;
    return (stat(name.c_str(), &exist) == 0);
}

void BackupLog(const std::string &message)
{
    FILE *fp = fopen(file_name.c_str(), "ab");
    if (fp == NULL) {
        perror("fopen error: ");
        assert(false);
    }

    int write_byte = fwrite(message.c_str(), sizeof(char), message.size(), fp);
    if (write_byte != message.size()) {
        perror("fwrite error: ");
        assert(false);
    }

    fflush(fp);
    fclose(fp);
}

int main(int args, char *argv[])
{
    if (args != 2) {
        Usage(argv[0]);
        perror("usage error");
        exit(-1);
    }

    uint16_t port = atoi(argv[1]);
    std::unique_ptr<TcpServer> tcp_server(new TcpServer(port, BackupLog));

    tcp_server->InitService();
    tcp_server->StartService();

    return 0;
}