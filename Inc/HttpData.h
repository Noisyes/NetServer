#pragma once

#include <memory>

#include "Timer.h"
#include "Socket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class TimerNode;

class HttpData : public std::enable_shared_from_this<HttpData>
{
public:
    HttpData() : epoll_fd(-1) {}
    std::shared_ptr<HttpRequest> httpRequest;
    std::shared_ptr<HttpResponse> httpResponse;
    std::shared_ptr<ClientSocket> clientSocket;
    int epoll_fd;
    void closeTimer();
    void setTimer(std::shared_ptr<TimerNode> timerNode);

private:
    std::weak_ptr<TimerNode> weak_timer;
};