#include "../../include/database/ConnectionGuard.hpp"
#include "../../include/database/ConnectionPool.hpp"

ConnectionGuard::ConnectionGuard(
    Database* db,
    ConnectionPool& pool
) : db_(db), pool_(pool) {}

ConnectionGuard::ConnectionGuard(ConnectionGuard&& other) noexcept
    : db_(other.db_), pool_(other.pool_) {
    other.db_ = nullptr;
}

ConnectionGuard::~ConnectionGuard() {
    if (db_) pool_.release(db_);
}

Database& ConnectionGuard::get() {
    return *db_;
}
