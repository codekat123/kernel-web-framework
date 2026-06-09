#include "../../include/router/AuthRoutes.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/database/ConnectionGuard.hpp"
#include "../../include/database/Database.hpp"
#include "../../include/database/Statement.hpp"
#include "../../include/auth/PasswordHasher.hpp"
#include "../../include/auth/JwtService.hpp"
#include "../../include/utils/JsonParser.hpp"

static const std::string SECRET_KEY = "be_proud_of_yourself";

void registerAuthRoutes(Router& router, ConnectionPool& pool) {
    router.addRoute("/register", [&pool](const HttpRequest& req) {
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
}
