#include "../../include/threading/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; i++) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
            
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);

                    condition.wait(lock, [this]() {
                        return !tasks.empty() || stop;
                    });

                    if (stop && tasks.empty()) {
                    return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        stop = true;
    }

    condition.notify_all();

    for (auto& worker : workers)
    {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
    
        tasks.push(task);
    }
    condition.notify_one();
}
