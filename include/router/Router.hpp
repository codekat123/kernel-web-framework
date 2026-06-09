#pragma once

#include <string>
#include <map>
#include <functional>

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"

class Router {
public:
    std::map<
        std::string,
        std::function<HttpResponse(const HttpRequest&)>
    > routes;

    void addRoute(
        const std::string& path,
        std::function<HttpResponse(const HttpRequest&)> handler
    );

    HttpResponse route(
        const std::string& path,
        const HttpRequest& req
    );
};;
