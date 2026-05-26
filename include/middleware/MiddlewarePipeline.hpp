#pragma once

#include <vector>
#include "Middleware.hpp"

class MiddlewarePipeline {
public:
    void use(Middleware mw);
    void execute(HttpRequest& req, HttpResponse& res);

private:
    std::vector<Middleware> middlewares;
};
