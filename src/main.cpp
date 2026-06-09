#include <filesystem>
#include <functional>
#include <iostream>

#include "../include/server/TcpServer.hpp"
#include "../include/router/Router.hpp"
#include "../include/middleware/Logger.hpp"
#include "../include/middleware/RequestTimer.hpp"
#include "../include/static/StaticFileHandler.hpp"
#include "../include/database/ConnectionPool.hpp"
#include "../include/database/ConnectionGuard.hpp"
#include "../include/database/Database.hpp"
#include "../include/router/HomeRoutes.hpp"
#include "../include/router/AuthRoutes.hpp"

int main() {
    try {
        ConnectionPool pool("app.db", 3);

        {
            ConnectionGuard guard = pool.acquire();
            Database& db = guard.get();

            db.execute(
                "CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "username TEXT NOT NULL UNIQUE,"
                "password_hash TEXT NOT NULL"
                ");"
            );
        }

        Router router;
        registerHomeRoutes(router);
        registerAuthRoutes(router, pool);

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
