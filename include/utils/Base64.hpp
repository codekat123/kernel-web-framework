#pragma once

#include <string>

class Base64 {
public:
    static std::string encode(const unsigned char* data, size_t len);
    static std::string decode(const std::string& input);
};
