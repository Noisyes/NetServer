#pragma once
#include <queue>
#include <deque>
#include <memory>
#include "Utils.h"
#include "mutex.h"
#include "HttpData.h"

class HttpData;

class TimerNode
{
public:
    explicit TimerNode(std::shared_ptr<HttpData> httpData, size_t timeout);
    ~TimerNode();
    bool isDeleted() const { return _deleted; }
    size_t getExpireTime() { return _expiredTime; }
    bool isExpire()
    {
        return currentTime < _expiredTime;
    }

    void deleted();

    static void current_time();

    static size_t currentTime;

    void (*cb_func)(std::shared_ptr<HttpData>);

private:
    bool _deleted;
    size_t _expiredTime;
    std::shared_ptr<HttpData> _httpData;
};

struct TimerCmp
{
    bool operator()(std::shared_ptr<TimerNode> &left, std::shared_ptr<TimerNode> &right)
    {
        return left->getExpireTime() > right->getExpireTime();
    }
};

class TimerManager
{
public:
    using shared_TimerNode = std::shared_ptr<TimerNode>;

public:
    void addTimer(std::shared_ptr<HttpData> http, size_t timeout);
    void deleteTimerNode();
    void handle_expired_event();

    const static size_t DEFAULT_TIME_OUT;

private:
    std::priority_queue<shared_TimerNode, std::deque<shared_TimerNode>, TimerCmp> _timerQueue;
    Mutex _queueMutex;
};