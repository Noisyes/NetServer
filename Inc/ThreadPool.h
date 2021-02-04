#pragma once

#include <memory>
#include <list>
#include <functional>
#include <exception>

#include "Utils.h"
#include "mutex.h"
#include "condition.h"
struct ThreadTask
{
    std::function<void(std::shared_ptr<void>)> process;
    std::shared_ptr<void> arg;
};

class ThreadPool
{
public:
    ThreadPool(int thread_num, int max_request);
    ~ThreadPool();
    bool append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)>);

private:
    static void *worker(void *arg);
    void run();

private:
    Mutex mutex;
    Condition cond;

    int _threadNum;
    int _maxRequestNum;
    pthread_t *_threads;
    std::list<ThreadTask> taskList;
    bool _shutdown;
};
