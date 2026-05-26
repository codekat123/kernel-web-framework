#include <memory>
#include <functional>
#include "../../include/middleware/MiddlewarePipeline.hpp"

void MiddlewarePipeline::use(Middleware mw) {
    middlewares.push_back(mw);
}

void MiddlewarePipeline::execute(
    HttpRequest& req,
    HttpResponse& res
) {
    auto index = std::make_shared<int>(0);
    auto next = std::make_shared<std::function<void()>>();

    *next = [this, &req, &res, index, next]() {
        if (*index >= (int)middlewares.size()) {
            return;
        }

        Middleware current = middlewares[(*index)++];
        current(req, res, *next);
    };

    (*next)();
}
