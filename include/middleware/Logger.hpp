#pragma once
#include "Middleware.hpp"

class Logger {
public:
    static void handle(
        HttpRequest& req,
        HttpResponse& res,
        std::function<void()> next
    );
};
