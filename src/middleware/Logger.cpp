#include <iostream>
#include "../../include/middleware/Logger.hpp"

void Logger::handle(
    HttpRequest& req,
    HttpResponse& res,
    std::function<void()> next
) {
    std::cout << "[LOG] " << req.method
              << " " << req.path << "\n";

    next(); 

    std::cout << "[LOG] Response: "
              << res.status_code << "\n";
}
