#include <iostream>

#include "../../include/http/HttpRequest.hpp"

void HttpRequest::print() const {
    std::cout << "Method: " << method << "\n";
    std::cout << "Path: " << path << "\n";
    std::cout << "Version: " << version << "\n";
}
