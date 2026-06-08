#include "../../include/middleware/JwtMiddleware.hpp"
#include "../../include/auth/JwtService.hpp"

Middleware JwtMiddleware::create(const std::string& secret) {
    return [secret](
        HttpRequest& req,
        HttpResponse& res,
        std::function<void()> next
    ) {
        auto it = req.headers.find("Authorization");

        if (it == req.headers.end()) {
            res.status_code = 401;
            res.body = "Unauthorized";
            return;
        }

        std::string auth = it->second;

        if (auth.substr(0, 7) != "Bearer ") {
            res.status_code = 401;
            res.body = "Unauthorized";
            return;
        }

        std::string token = auth.substr(7);

        try {
            JwtService::verify(token, secret);
        } catch (const std::exception&) {
            res.status_code = 401;
            res.body = "Unauthorized";
            return;
        }

        next();
    };
}
