#pragma once
#include "CommonHead.h"

class ClientSocket;

class ServerSocket
{
public:
    ServerSocket(int port = DEFAULT_PORT, const char *ip = nullptr);
    ~ServerSocket();

    bool bind();
    bool listen();
    int accept(ClientSocket &) const;
    void close();
    int getepollfd() const { return _epollfd; }
    int _listenfd;
    int _epollfd;

private:
    sockaddr_in _addr;

    int _port;
    const char *_ip;
};

class ClientSocket
{
public:
    ClientSocket() : clientfd(-1) {}
    void close();
    ~ClientSocket();

public:
    int clientfd;
    sockaddr_in _addr;
};