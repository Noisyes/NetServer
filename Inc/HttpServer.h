#pragma once

#include <memory>
#include "Socket.h"
#include "HttpData.h"

#define BUFFER_SIZE 4096

class HttpServer
{
public:
    enum FileStatus
    {
        FILE_OK = 0,
        FILE_NOT_FOUND,
        FILE_FORBIDEN
    };
    using spHttpData = std::shared_ptr<HttpData>;

public:
    explicit HttpServer(int port = DEFAULT_PORT, const char *ip = nullptr) : serverSocket(port, ip)
    {
        assert(serverSocket.bind());
        assert(serverSocket.listen());
    }

    void run(int thread_num, int max_request);
    void do_request(std::shared_ptr<void> arg);

private:
    void header(spHttpData);
    FileStatus static_file(spHttpData, const char *);
    void send(spHttpData, FileStatus);
    void getMime(spHttpData);
    ServerSocket serverSocket;
};