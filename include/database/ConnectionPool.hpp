#pragma once 

#include <queue>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>

#include "Database.hpp"
#include "ConnectionGuard.hpp"


class ConnectionPool {
public:
    ConnectionPool(const std::string& db_path, int pool_size);
    ~ConnectionPool();

    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    ConnectionGuard acquire();
    void release(Database* db);
private:
    std::vector<std::unique_ptr<Database>> connections_;
    std::queue<Database*> available_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
