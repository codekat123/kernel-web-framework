#include <iostream>
#include <chrono>
#include "../../include/middleware/RequestTimer.hpp"

void RequestTimer::handle(
    HttpRequest& req,
    HttpResponse& res,
    std::function<void()> next
) {
    auto start = std::chrono::high_resolution_clock::now();

    next();

    auto end = std::chrono::high_resolution_clock::now();

    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "[TIMER] " << us << " microseconds\n";
}
