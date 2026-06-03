#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct sqlite3_stmt;

using Row      = std::unordered_map<std::string, std::string>;
using ResultSet = std::vector<Row>;

class Statement
{
public:
    explicit Statement(sqlite3_stmt* stmt);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    void bind(int index, const std::string& value);
    void bind(int index, int value);

    bool step();

    Row         currentRow();   
    ResultSet   fetchAll();    

private:
    sqlite3_stmt* stmt_;
};;
