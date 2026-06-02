#pragma once

struct sqlite3_stmt;

class Statement
{
public:
    explicit Statement(
        sqlite3_stmt* stmt
    );

    ~Statement();

    Statement(
        const Statement&
    ) = delete;

    Statement& operator=(
        const Statement&
    ) = delete;

private:
    sqlite3_stmt* stmt_;
};
