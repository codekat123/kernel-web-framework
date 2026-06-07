#include "../../include/database/Database.hpp"

#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
Database::Database(
    const std::string& path
)
    : connection_(nullptr)
{
    int rc =
        sqlite3_open(
            path.c_str(),
            &connection_
        );

    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(
                connection_
            )
        );
    }
}

Database::~Database()
{
    if (connection_)
    {
        sqlite3_close(
            connection_
        );
    }
}

bool Database::execute(
    const std::string& sql
)
{
    char* error = nullptr;

    int rc =
        sqlite3_exec(
            connection_,
            sql.c_str(),
            nullptr,
            nullptr,
            &error
        );

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << (error ? error : "unknown") << "\n";
        sqlite3_free(error);
        return false;
    }

    return true;
}

Statement Database::prepare(const std::string& sql)
{
    sqlite3_stmt* stmt = nullptr;

    int result = sqlite3_prepare_v2(
        connection_,
        sql.c_str(),
        -1,
        &stmt,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(connection_)
        );
    }

    return Statement(stmt);
}
