#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
    std::vector<std::thread> workers;

    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;

    std::condition_variable condition;

    bool stop = false;

public:
    ThreadPool(size_t thread_count);
    ~ThreadPool();
    void enqueue(std::function<void()> task);
};
