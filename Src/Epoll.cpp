#include "Epoll.h"

std::unordered_map<int, std::shared_ptr<HttpData>> Epoll::httpDataMap;
const int Epoll::MAX_EVENTS = 10000;
epoll_event *Epoll::events;
const __uint32_t Epoll::DEFAULT_EVENT = (EPOLLIN | EPOLLET | EPOLLONESHOT);
TimerManager Epoll::timerManager;

int Epoll::init(int max_event)
{
    int epollfd = ::epoll_create(max_event);
    if (epollfd == -1)
    {
        std::cout << "epoll create error" << std::endl;
        exit(-1);
    }
    events = new epoll_event[max_event];
    return epollfd;
}

bool Epoll::addFD(int epoll_fd, int fd, __uint32_t addEvents, std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = addEvents;
    event.data.fd = fd;

    httpDataMap[fd] = httpData;

    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0)
    {
        std::cout << "epoll add error :" << errno << std::endl;
        httpDataMap[fd].reset();
        return false;
    }
    return true;
}
bool Epoll::modFD(int epoll_fd, int fd, __uint32_t modEvents, std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = modEvents;
    event.data.fd = fd;

    httpDataMap[fd] = httpData;

    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    if (ret < 0)
    {
        std::cout << "epoll mod error :" << errno << std::endl;
        httpDataMap[fd].reset();
        return false;
    }
    return true;
}
bool Epoll::delFD(int epoll_fd, int fd, __uint32_t delEvents)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = delEvents;

    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
    if (ret < 0)
    {
        std::cout << "epoll delete error :" << errno << std::endl;
        return false;
    }
    auto iter = httpDataMap.find(fd);
    if (iter != httpDataMap.end())
    {
        httpDataMap[fd].reset();
        httpDataMap.erase(iter);
    }
    return true;
}

std::vector<std::shared_ptr<HttpData>>
Epoll::poll(const ServerSocket &serverSocket, int max_event, int timeout)
{
    int event_nums = epoll_wait(serverSocket.getepollfd(), events, max_event, timeout);
    if (event_nums < 0)
    {
        std::cout << "epoll_num = " << event_nums << std::endl;
        std::cout << "epoll_wait error " << errno << std::endl;
    }
    std::vector<std::shared_ptr<HttpData>> httpDatas;
    for (int i = 0; i < event_nums; i++)
    {
        int fd = events[i].data.fd;
        if (fd == serverSocket._listenfd)
        {
            handleConnection(serverSocket);
        }
        else if (events[i].events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP))
        {
            //TODO 描述符出错   移除定时器 关闭文件描述符
            auto iter = httpDataMap.find(fd);
            if (iter != httpDataMap.end())
            {
                iter->second->closeTimer();
                ::close(fd);
            }
        }
        else if (events[i].events & (EPOLLIN))
        {
            //TODO 来读事件
            auto iter = httpDataMap.find(fd);
            if (iter != httpDataMap.end())
            {
                httpDatas.push_back(iter->second);
                httpDataMap[fd]->closeTimer();
                //timerManager.addTimer(httpDataMap[fd], TimerManager::DEFAULT_TIME_OUT);
                httpDataMap.erase(iter);
            }
            else
            {
                std::cout << "长连接第二次连接没找到" << std::endl;
                ::close(fd);
            }
        }
        else if (events[i].events & (EPOLLOUT))
        {
            //TODO  来写时间
        }
    }
    return httpDatas;
}
bool Epoll::handleConnection(const ServerSocket &serverSocket)
{
    std::shared_ptr<ClientSocket> tmpSocket(new ClientSocket());
    while (serverSocket.accept(*tmpSocket) > 0)
    {
        int ret = setnonblocking(tmpSocket->clientfd);
        std::shared_ptr<HttpData> spHttpData(new HttpData);
        spHttpData->httpRequest = std::shared_ptr<HttpRequest>(new HttpRequest());
        spHttpData->httpResponse = std::shared_ptr<HttpResponse>(new HttpResponse());
        std::shared_ptr<ClientSocket> clientSocket(new ClientSocket);
        clientSocket.swap(tmpSocket);

        spHttpData->clientSocket = clientSocket;
        spHttpData->epoll_fd = serverSocket.getepollfd();
        addFD(serverSocket.getepollfd(), clientSocket->clientfd, DEFAULT_EVENT, spHttpData);
        timerManager.addTimer(spHttpData, TimerManager::DEFAULT_TIME_OUT);
    }
    return true;
}
