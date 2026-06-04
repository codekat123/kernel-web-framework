#include "../../include/database/ConnectionGuard.hpp"
#include "../../include/database/ConnectionPool.hpp"



ConnectionGuard::ConnectionGuard(
    Database* db,
    ConnectionPool& pool
) : db_(db), pool_(pool) {}

ConnectionGuard::~ConnectionGuard() {
    pool_.release(db_);
}

Database& ConnectionGuard::get() {
    return *db_;
}
