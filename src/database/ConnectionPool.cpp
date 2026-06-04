#include "../../include/database/ConnectionPool.hpp"
#include "../../include/database/ConnectionGuard.hpp"




ConnectionPool::ConnectionPool(
    const std::string& db_path,
    int pool_size
) {
    for (int i = 0; i < pool_size; i++) {
        connections_.push_back(
            std::make_unique<Database>(db_path)
        );
        available_.push(connections_.back().get());
    }
}

ConnectionPool::~ConnectionPool() {}





ConnectionGuard ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock, [this] {
        return !available_.empty();
    });

    Database* db = available_.front();
    available_.pop();

    return ConnectionGuard(db, *this);
}


void ConnectionPool::release(Database* db) {
    std::unique_lock<std::mutex> lock(mutex_);
    available_.push(db);
    cv_.notify_one();
}
