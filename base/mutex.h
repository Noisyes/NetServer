#pragma once
#include <pthread.h>
#include "noncopyable.h"

class Mutex : noncopyable
{
public:
    Mutex() { mutex_ = PTHREAD_MUTEX_INITIALIZER; }
    ~Mutex() { pthread_mutex_destroy(&mutex_); }
    void lock() { pthread_mutex_lock(&mutex_); }
    void unlock() { pthread_mutex_unlock(&mutex_); }
    pthread_mutex_t *get() { return &mutex_; }

private:
    pthread_mutex_t mutex_;
    friend class Condition;
};

class MutexGuard : noncopyable
{
public:
    explicit MutexGuard(Mutex &mutex) : mutex_(mutex)
    {
        mutex_.lock();
    }
    ~MutexGuard()
    {
        mutex_.unlock();
    }

private:
    Mutex &mutex_;
};