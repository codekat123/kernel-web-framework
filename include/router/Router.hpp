#pragma once

#include <string>
#include <map>
#include <functional>

#include "../http/HttpResponse.hpp"

class Router {
public:
    std::map<
        std::string,
        std::function<HttpResponse()>
    > routes;

    void addRoute(
        const std::string& path,
        std::function<HttpResponse()> handler
    );

    HttpResponse route(
        const std::string& path
    );
};
