#include "ThreadProvider.h"
#include <iostream>
#include <chrono>

int ThreadProvider::getMaxQueueLength() const
{
    return max_queue_len;
}

void ThreadProvider::setMaxQueueLength(int value)
{
    max_queue_len = value;
}

ThreadProvider::~ThreadProvider()
{
    stop();
}

void ThreadProvider::push(const FramePtrWrapper &d)
{
    if (is_exit)
        return;
    std::lock_guard<std::mutex> lock(mutex);
    while (curQueueSize >= max_queue_len)
    {
        data_queue.pop_front();
        --curQueueSize;
    }
    data_queue.emplace_back(d);
    ++curQueueSize;
}

void ThreadProvider::push(FramePtrWrapper &&d)
{
    if (is_exit)
        return;
    std::lock_guard<std::mutex> lock(mutex);
    while (curQueueSize >= max_queue_len)
    {
        data_queue.pop_front();
        --curQueueSize;
    }
    data_queue.emplace_back(d);
    ++curQueueSize;
}

FramePtrWrapper ThreadProvider::pop()
{
    if (is_exit)
        return FramePtrWrapper();
    std::lock_guard<std::mutex> lock(mutex);
    if (data_queue.empty())
    {
        return FramePtrWrapper();
    }
    FramePtrWrapper d;
    d.swap(data_queue.front());
    data_queue.pop_front();
    --curQueueSize;
    return d;
}

FramePtrWrapper ThreadProvider::top()
{
    if (is_exit)
        return FramePtrWrapper();
    std::lock_guard<std::mutex> lock(mutex);
    if (data_queue.empty())
    {
        return FramePtrWrapper();
    }
    FramePtrWrapper d(data_queue.front());
    return d;
}

void ThreadProvider::start()
{
    std::lock_guard<std::mutex> lock(mutex);
    is_exit = false;
    m_thread = std::thread(&ThreadProvider::run, this);
}

void ThreadProvider::stop()
{
    std::lock_guard<std::mutex> lock(mutex);
    is_exit = true;
    if (m_thread.joinable())
        m_thread.join();
    data_queue.clear();
    curQueueSize = 0;
}
