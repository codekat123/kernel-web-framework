#include "../../include/router/Router.hpp"

void Router::addRoute(
    const std::string& path,
    std::function<HttpResponse()> handler
) {
    routes[path] = handler;
}

HttpResponse Router::route(
    const std::string& path
) {
    if (routes.find(path) != routes.end()) {
        return routes[path]();
    }

    HttpResponse response;

    response.status_code = 404;
    response.body = "404 Not Found";

    return response;
}
