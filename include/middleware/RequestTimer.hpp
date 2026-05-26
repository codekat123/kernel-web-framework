#pragma once
#include "Middleware.hpp"

class RequestTimer {
public:
    static void handle(
        HttpRequest& req,
        HttpResponse& res,
        std::function<void()> next
    );
};
