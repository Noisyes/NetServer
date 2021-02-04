#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "Socket.h"
#include "HttpData.h"
#include "Utils.h"

class Epoll
{
public:
    static int init(int max_event);
    static bool addFD(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<HttpData> httpData);
    static bool modFD(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<HttpData> httpData);
    static bool delFD(int epoll_fd, int fd, __uint32_t events);

    static std::vector<std::shared_ptr<HttpData>>
    poll(const ServerSocket &serverSocket, int max_event, int timeout);
    static bool handleConnection(const ServerSocket &serverSocket);

public:
    static std::unordered_map<int, std::shared_ptr<HttpData>> httpDataMap;
    static const int MAX_EVENTS;
    static epoll_event *events;
    const static __uint32_t DEFAULT_EVENT;
    static TimerManager timerManager;
};