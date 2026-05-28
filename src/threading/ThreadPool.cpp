#include "threading/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; i++) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
            }
        });
    }
}
