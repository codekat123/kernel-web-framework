#pragma once

#include <string>

class Hmac {
public:
    static std::string sha256(const std::string& message, const std::string& secret);
};
