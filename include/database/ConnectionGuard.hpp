#pragma once

#include "Database.hpp"


class ConnectionPool;

class ConnectionGuard {
public:
    ConnectionGuard(Database* db, ConnectionPool& pool);
    ConnectionGuard(ConnectionGuard&& other) noexcept;
    ~ConnectionGuard();

    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;

    Database& get();

private:
    Database* db_;
    ConnectionPool& pool_;
};
