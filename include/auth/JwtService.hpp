#pragma once

#include <string>

class JwtService {
public:
    static std::string issue(const std::string& user_id, const std::string& secret);
    static std::string verify(const std::string& token, const std::string& secret);
};
