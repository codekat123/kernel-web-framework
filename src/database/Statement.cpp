#include "../../include/database/Statement.hpp"
#include <stdexcept>
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


void Statement::bind(int index, const std::string& value)
{
    int result = sqlite3_bind_text(
        stmt_,
        index,
        value.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind text parameter"
        );
    }
}

void Statement::bind(int index, int value)
{
    int result = sqlite3_bind_int(
        stmt_,
        index,
        value
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind int parameter"
        );
    }
}

bool Statement::step()
{
    int result = sqlite3_step(stmt_);

    if (result == SQLITE_ROW)
    {
        return true;
    }

    if (result == SQLITE_DONE)
    {
        return false;
    }

    throw std::runtime_error(
        "sqlite3_step() failed"
    );
}

Row Statement::currentRow()
{
    Row row;

    int columns = sqlite3_column_count(stmt_);

    for (int i = 0; i < columns; i++)
    {
        std::string name = sqlite3_column_name(stmt_, i);

        const unsigned char* raw = sqlite3_column_text(stmt_, i);

        std::string value = raw
            ? reinterpret_cast<const char*>(raw)
            : "";

        row[name] = value;
    }

    return row;
}

ResultSet Statement::fetchAll()
{
    ResultSet results;

    while (step())
    {
        results.push_back(currentRow());
    }

    return results;
}
