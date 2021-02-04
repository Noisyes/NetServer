#include "Socket.h"
#include "Utils.h"
ServerSocket::ServerSocket(int port, const char *ip) : _port(port), _ip(ip)
{
    memset(&_addr, 0, sizeof(sockaddr_in));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    if (ip != nullptr)
    {
        //inet_pton(AF_INET, ip, &_addr.sin_addr);
        _addr.sin_addr.s_addr = inet_addr(ip);
    }
    else
    {
        _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    _listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (_listenfd == -1)
    {
        std::cout << "create listenfd error in file "
                  << "<" << __FILE__ << ">"
                  << " at " << __LINE__ << std::endl;
        exit(0);
    }

    setReusePort(_listenfd);
    setnonblocking(_listenfd);
}

bool ServerSocket::bind()
{
    int ret = ::bind(_listenfd, (struct sockaddr *)&_addr, sizeof(_addr));
    if (ret == -1)
    {
        std::cout << "bind  error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        return false;
    }
    return true;
}

bool ServerSocket::listen()
{
    int ret = ::listen(_listenfd, 1024);
    if (ret < 0)
    {
        std::cout << "listen error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        return false;
    }
    return true;
}

int ServerSocket::accept(ClientSocket &clientSocket) const
{
    int clientfd = ::accept(_listenfd, NULL, NULL);
    if (clientfd < 0)
    {
        if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            return clientfd;
        std::cout << "accept error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        std::cout << "clientfd:" << clientfd << std::endl;
    }
    std::cout << "accept a client : " << clientfd << std::endl;
    clientSocket.clientfd = clientfd;
    return clientfd;
}

void ServerSocket::close()
{
    if (_listenfd >= 0)
    {
        ::close(_listenfd);
        std::cout << "定时器超时关闭, 文件描述符:" << _listenfd << std::endl;
        _listenfd = -1;
    }
}

ServerSocket::~ServerSocket()
{
    close();
}

void ClientSocket::close()
{
    if (clientfd >= 0)
    {
        std::cout << "文件描述符关闭: " << clientfd << std::endl;
        ::close(clientfd);
        clientfd = -1;
    }
}

ClientSocket::~ClientSocket()
{
    close();
}