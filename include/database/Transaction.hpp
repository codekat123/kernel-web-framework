#pragma once

class Database;

class Transaction
{
public:
    explicit Transaction(
        Database& db
    );

    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit();
    void rollback();

private:
    Database& db_;
    bool committed_;
};
