#include "ThreadPool.h"
#include <iostream>
#include <sys/prctl.h>

ThreadPool::ThreadPool(int thread_num, int max_request) : _threadNum(thread_num), _maxRequestNum(max_request),
                                                          _shutdown(false), _threads(nullptr), cond(mutex)
{
    if ((thread_num <= 0) || max_request <= 0)
    {
        throw std::exception();
    }
    _threads = new pthread_t[thread_num];
    if (!_threads)
    {
        throw std::exception();
    }
    for (int i = 0; i < thread_num; i++)
    {
        std::cout << "create the " << i + 1 << " thread" << std::endl;
        if (pthread_create(_threads + i, NULL, worker, this) != 0)
        {
            delete[] _threads;
            throw std::exception();
        }

        if (pthread_detach(_threads[i]))
        {
            delete[] _threads;
            throw std::exception();
        }
    }
}

ThreadPool::~ThreadPool()
{
    delete[] _threads;
    _shutdown = true;
}

bool ThreadPool::append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> func)
{
    if (_shutdown)
    {
        std::cout << "threadpool has shutdown" << std::endl;
        return false;
    }
    ThreadTask task;
    task.arg = arg;
    task.process = func;

    mutex.lock();
    if (taskList.size() > _maxRequestNum)
    {
        mutex.unlock();
        return false;
    }
    taskList.push_back(task);
    mutex.unlock();
    cond.notify();
    return true;
}

void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    if (!pool)
        return nullptr;
    prctl(PR_SET_NAME, "EventLoopThread");
    pool->run();
    return pool;
}

void ThreadPool::run()
{
    while (!_shutdown)
    {
        mutex.lock();
        cond.wait();
        if (taskList.empty())
        {
            mutex.unlock();
            continue;
        }
        ThreadTask curTask = taskList.front();
        taskList.pop_front();
        mutex.unlock();

        curTask.process(curTask.arg);
    }
}
