#include <sstream>

#include "../../include/http/HttpParser.hpp"

HttpRequest HttpParser::parse(
    const std::string& raw_request
) {
    HttpRequest request;

    std::istringstream request_stream(
        raw_request
    );

    request_stream
        >> request.method
        >> request.path
        >> request.version;

	std::string line;
// consume leftover newline after >>
    std::getline(request_stream, line);

    while (std::getline(request_stream, line)) {

        if (line == "\r" || line.empty()) {
            break;
        }

        size_t separator = line.find(':');

        if (separator != std::string::npos) {

            std::string key =
                line.substr(0, separator);

            std::string value =
                line.substr(separator + 1);

            if (!value.empty() &&
                value[0] == ' ') {

                value.erase(0, 1);
            }

            if (!value.empty() &&
                value.back() == '\r') {

                value.pop_back();
            }

            request.headers[key] = value;
        }
    }

    return request;
}
