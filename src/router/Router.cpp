#include "../../include/router/Router.hpp"

void Router::addRoute(
    const std::string& path,
    std::function<HttpResponse(const HttpRequest&)> handler
) {
    routes[path] = handler;
}

HttpResponse Router::route(
    const std::string& path,
    const HttpRequest& req
) {
    if (routes.find(path) != routes.end()) {
        return routes[path](req); 
    }

    HttpResponse response;
    response.status_code = 404;
    response.body = "404 Not Found";
    return response;
}
