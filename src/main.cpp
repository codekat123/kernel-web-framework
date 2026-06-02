#include <filesystem>
#include <functional>
#include <iostream>

#include "../include/server/TcpServer.hpp"
#include "../include/http/HttpResponse.hpp"
#include "../include/http/HttpRequest.hpp"
#include "../include/router/Router.hpp"
#include "../include/middleware/Logger.hpp"
#include "../include/middleware/RequestTimer.hpp"
#include "../include/static/StaticFileHandler.hpp"

HttpResponse homeHandler() {
    HttpResponse response;
    response.body = "Home Page";
    return response;
}

HttpResponse helloHandler() {
    HttpResponse response;
    response.body = "Hello From Router";
    return response;
}

int main() {
    try {
        Router router;
        router.addRoute("/", homeHandler);
        router.addRoute("/hello", helloHandler);

        TcpServer server(8080, router);

        StaticFileHandler staticFiles("public");

        server.use(Logger::handle);
        server.use(RequestTimer::handle);

        server.use([&staticFiles](
            HttpRequest& req,
            HttpResponse& res,
            std::function<void()> next
        ) {
            if (req.method != "GET") {
                next();
                return;
            }

            std::string file_path = "public" + req.path;
            if (std::filesystem::exists(file_path) &&
                std::filesystem::is_regular_file(file_path)) {
                res = staticFiles.serve(req.path);
            } else {
                next();
            }
        });

        server.start();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}
