#include "../../include/auth/JwtService.hpp"
#include "../../include/utils/Base64.hpp"
#include "../../include/utils/Hmac.hpp"
#include <stdexcept>
#include <chrono>

std::string JwtService::issue(const std::string& user_id, const std::string& secret) {
    std::string header_json = R"({"alg":"HS256","typ":"JWT"})";

    std::string header = Base64::encode(
        (const unsigned char*)header_json.c_str(),
        header_json.size()
    );

    auto now = std::chrono::system_clock::now();
    auto exp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count() + 3600;

    std::string payload_json =
        R"({"sub":")" + user_id + R"(","exp":)" + std::to_string(exp) + "}";

    std::string payload = Base64::encode(
        (const unsigned char*)payload_json.c_str(),
        payload_json.size()
    );

    std::string signature = Hmac::sha256(header + "." + payload, secret);

    return header + "." + payload + "." + signature;
}

std::string JwtService::verify(const std::string& token, const std::string& secret) {
    size_t first = token.find('.');
    size_t second = token.find('.', first + 1);

    if (first == std::string::npos || second == std::string::npos) {
        throw std::runtime_error("Invalid token format");
    }

    std::string header = token.substr(0, first);
    std::string payload = token.substr(first + 1, second - first - 1);
    std::string signature = token.substr(second + 1);

    std::string expected = Hmac::sha256(header + "." + payload, secret);

    if (expected != signature) {
        throw std::runtime_error("Invalid token signature");
    }

    std::string payload_json = Base64::decode(payload);

    size_t exp_pos = payload_json.find("\"exp\":");

    if (exp_pos == std::string::npos) {
        throw std::runtime_error("Missing exp claim");
    }

    long long exp = std::stoll(payload_json.substr(exp_pos + 6));

    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    if (now > exp) {
        throw std::runtime_error("Token expired");
    }

    size_t sub_pos = payload_json.find("\"sub\":\"");

    if (sub_pos == std::string::npos) {
        throw std::runtime_error("Missing sub claim");
    }

    size_t sub_start = sub_pos + 7;
    size_t sub_end = payload_json.find('"', sub_start);

    return payload_json.substr(sub_start, sub_end - sub_start);
}
