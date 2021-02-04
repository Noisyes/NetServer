#include "HttpData.h"

void HttpData::closeTimer()
{
    if (weak_timer.lock())
    {
        std::shared_ptr<TimerNode> tempNode(weak_timer.lock());
        tempNode->deleted();
        weak_timer.reset();
    }
}

void HttpData::setTimer(std::shared_ptr<TimerNode> timerNode)
{
    weak_timer = timerNode;
}
