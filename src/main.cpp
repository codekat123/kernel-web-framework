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
#include "../include/database/ConnectionPool.hpp"
#include "../include/middleware/JwtMiddleware.hpp"
#include "../include/auth/PasswordHasher.hpp"
#include "../include/auth/JwtService.hpp"

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
        ConnectionPool pool("test.db", 3);
        {    
            ConnectionGuard guard = pool.acquire();
            Database& db = guard.get();

            db.execute(
                "CREATE TABLE IF NOT EXISTS users "
                "(id INTEGER PRIMARY KEY, name TEXT);"
            );
            // db.execute(
            //     "INSERT INTO users (name) VALUES ('Ahmed');"
            // );
            //
            auto stmt = db.prepare("SELECT * FROM users;");
            auto rows = stmt.fetchAll();

            for (const auto& row : rows) {
                std::cout << "id: " << row.at("id")
                          << " name: " << row.at("name")
                          << "\n";
            }
        } // destroying the guard here, whoever read this object is now out of scope  
        Router router;
        router.addRoute("/", homeHandler);
        router.addRoute("/hello", helloHandler);

        TcpServer server(8080, router);
        StaticFileHandler staticFiles("public");
        

        std::string secret_key = "be_pround_of_your_self";
        std::string stored = PasswordHasher::hash("mypassword");
        std::cout << "Stored: " << stored << "\n";
        std::cout << "Valid: " << PasswordHasher::verify("mypassword", stored) << "\n";
        std::cout << "Invalid: " << PasswordHasher::verify("wrongpassword", stored) << "\n";

        // test JwtService
        std::string token = JwtService::issue("42", secret_key);
        std::cout << "Token: " << token << "\n";
        std::cout << "User id: " << JwtService::verify(token, secret_key) << "\n";
        

        server.use(Logger::handle);
        server.use(RequestTimer::handle);
        server.use(JwtMiddleware::create(secret_key));
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
