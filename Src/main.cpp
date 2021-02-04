#include <iostream>
#include <CommonHead.h>
#include <Epoll.h>
#include <Socket.h>
#include <HttpServer.h>

std::string basePath = ".";

int main()
{
    HttpServer server;
    server.run(4, 10000);
    return 0;
}