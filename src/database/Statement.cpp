#include "../../include/database/Statement.hpp"

#include <sqlite3.h>

Statement::Statement(
    sqlite3_stmt* stmt
)
    : stmt_(stmt)
{
}

Statement::~Statement()
{
    if (stmt_)
    {
        sqlite3_finalize(stmt_);
    }
}
