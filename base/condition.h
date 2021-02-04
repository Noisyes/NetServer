#pragma once
#include "mutex.h"
#include "noncopyable.h"

class Condition : noncopyable
{
public:
    Condition(Mutex &mutex) : mutex_(mutex)
    {
        pthread_cond_init(&cond, NULL);
    }
    ~Condition()
    {
        pthread_cond_destroy(&cond);
    }
    void wait()
    {
        pthread_cond_wait(&cond, mutex_.get());
    }
    void notify()
    {
        pthread_cond_signal(&cond);
    }
    void notifyAll()
    {
        pthread_cond_broadcast(&cond);
    }

private:
    Mutex &mutex_;
    pthread_cond_t cond;
};