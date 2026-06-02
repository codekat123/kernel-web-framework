#include "../../include/database/Database.hpp"

#include <sqlite3.h>
#include <stdexcept>

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
        sqlite3_free(error);
        return false;
    }

    return true;
}
