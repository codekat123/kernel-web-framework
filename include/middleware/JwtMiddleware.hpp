#pragma once

#include "Middleware.hpp"
#include <string>

class JwtMiddleware {
public:
    static Middleware create(const std::string& secret);
};
