#include "Timer.h"
#include "CommonHead.h"
#include "Epoll.h"
#include <sys/time.h>

size_t TimerNode::currentTime = 0;

const size_t TimerManager::DEFAULT_TIME_OUT = TIME_OUT * 1000;

void inline TimerNode::current_time()
{
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    currentTime = curTime.tv_sec * 1000 + curTime.tv_usec / 1000;
}

TimerNode::TimerNode(std::shared_ptr<HttpData> httpData, size_t timeout) : _deleted(false), _httpData(httpData)
{
    current_time();
    _expiredTime = currentTime + timeout;
}

void TimerNode::deleted()
{
    _httpData.reset();
    _deleted = true;
}

TimerNode::~TimerNode()
{
    if (_httpData)
    {
        auto iter = Epoll::httpDataMap.find(_httpData->clientSocket->clientfd);
        if (iter != Epoll::httpDataMap.end())
        {
            Epoll::httpDataMap.erase(iter);
        }
    }
}

void TimerManager::addTimer(std::shared_ptr<HttpData> httpData, size_t timeout)
{
    shared_TimerNode timerNode(new TimerNode(httpData, timeout));

    {
        MutexGuard mutex(_queueMutex);
        _timerQueue.push(timerNode);
        httpData->setTimer(timerNode);
    }
}

void TimerManager::handle_expired_event()
{
    MutexGuard mutex(_queueMutex);
    TimerNode::current_time();
    while (!_timerQueue.empty())
    {
        shared_TimerNode tmpNode = _timerQueue.top();
        if (tmpNode->isDeleted())
        {
            _timerQueue.pop();
        }
        else if (!tmpNode->isExpire())
        {
            _timerQueue.pop();
        }
        else
        {
            break;
        }
    }
}
