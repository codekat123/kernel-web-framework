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
#include "../include/database/ConnectionGuard.hpp"
#include "../include/middleware/JwtMiddleware.hpp"
#include "../include/auth/PasswordHasher.hpp"
#include "../include/auth/JwtService.hpp"
#include "../include/utils/JsonParser.hpp"

const std::string SECRET_KEY = "be_proud_of_yourself";

HttpResponse homeHandler(const HttpRequest& req) {
    HttpResponse response;
    response.body = "Home Page";
    return response;
}

HttpResponse helloHandler(const HttpRequest& req) {
    HttpResponse response;
    response.body = "Hello From Router";
    return response;
}

// registerHandler and loginHandler need access to the pool
// so we build them as lambdas inside main() where pool is in scope

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

        router.addRoute("/", homeHandler);
        router.addRoute("/hello", helloHandler);

        router.addRoute("/register", [&pool](const HttpRequest& req) {
            HttpResponse res;
            res.content_type = "application/json";

            // only accept POST
            if (req.method != "POST") {
                res.status_code = 405;
                res.body = R"({"error": "method not allowed"})";
                return res;
            }

            auto fields = parseJson(req.body);

            // make sure both fields exist
            if (fields.find("username") == fields.end() ||
                fields.find("password") == fields.end()) {
                res.status_code = 400;
                res.body = R"({"error": "username and password required"})";
                return res;
            }

            std::string username = fields["username"];
            std::string password = fields["password"];

            std::string password_hash = PasswordHasher::hash(password);

            ConnectionGuard guard = pool.acquire();
            Database& db = guard.get();

            auto stmt = db.prepare(
                "INSERT INTO users (username, password_hash) VALUES (?, ?);"
            );

            stmt.bind(1, username);
            stmt.bind(2, password_hash);

            try {
                stmt.step();
            } catch (const std::exception&) {
                // step() throws if the insert fails
                // most likely cause: username already exists (UNIQUE constraint)
                res.status_code = 409;
                res.body = R"({"error": "username already taken"})";
                return res;
            }

            res.status_code = 201;
            res.body = R"({"message": "user created"})";
            return res;
        });

        router.addRoute("/login", [&pool](const HttpRequest& req) {
            HttpResponse res;
            res.content_type = "application/json";

            if (req.method != "POST") {
                res.status_code = 405;
                res.body = R"({"error": "method not allowed"})";
                return res;
            }

            auto fields = parseJson(req.body);

            if (fields.find("username") == fields.end() ||
                fields.find("password") == fields.end()) {
                res.status_code = 400;
                res.body = R"({"error": "username and password required"})";
                return res;
            }

            std::string username = fields["username"];
            std::string password = fields["password"];

            ConnectionGuard guard = pool.acquire();
            Database& db = guard.get();

            auto stmt = db.prepare(
                "SELECT id, password_hash FROM users WHERE username = ?;"
            );

            stmt.bind(1, username);

            // step() returns true if a row was found
            if (!stmt.step()) {
                res.status_code = 401;
                res.body = R"({"error": "invalid credentials"})";
                return res;
            }

            Row row = stmt.currentRow();

            std::string stored_hash = row["password_hash"];
            std::string user_id     = row["id"];

            if (!PasswordHasher::verify(password, stored_hash)) {
                res.status_code = 401;
                res.body = R"({"error": "invalid credentials"})";
                return res;
            }

            std::string token = JwtService::issue(user_id, SECRET_KEY);

            res.status_code = 200;
            res.body = R"({"token": ")" + token + R"("})";
            return res;
        });

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
