#pragma once

#include <string>
#include "Statement.hpp"
struct sqlite3;

class Database
{
public:
    explicit Database(
        const std::string& path
    );

    ~Database();

    Database(const Database&) = delete;
    Database& operator=(
        const Database&
    ) = delete;

    bool execute(
        const std::string& sql
    );
    
    Statement prepare(
    const std::string& sql
    );

private:
    sqlite3* connection_;
};
