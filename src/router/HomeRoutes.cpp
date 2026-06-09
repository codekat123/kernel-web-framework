#include "../../include/router/HomeRoutes.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"

void registerHomeRoutes(Router& router) {
    router.addRoute("/", [](const HttpRequest& req) {
        HttpResponse res;
        res.body = "Home Page";
        return res;
    });

    router.addRoute("/hello", [](const HttpRequest& req) {
        HttpResponse res;
        res.body = "Hello From Router";
        return res;
    });
}
