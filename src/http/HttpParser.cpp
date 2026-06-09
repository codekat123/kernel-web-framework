#include <sstream>

#include "../../include/http/HttpParser.hpp"

HttpRequest HttpParser::parse(
    const std::string& raw_request
) {
    HttpRequest request;

    std::istringstream request_stream(raw_request);

    request_stream
        >> request.method
        >> request.path
        >> request.version;

    std::string line;
    std::getline(request_stream, line); // consume leftover newline after >>

    while (std::getline(request_stream, line)) {

        if (line == "\r" || line.empty()) {
            break; // blank line = end of headers, body starts after this
        }

        size_t separator = line.find(':');

        if (separator != std::string::npos) {

            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 1);

            if (!value.empty() && value[0] == ' ') {
                value.erase(0, 1);
            }

            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }

            request.headers[key] = value;
        }
    }

    // read body if Content-Length header is present
    auto it = request.headers.find("Content-Length");

    if (it != request.headers.end()) {

        int content_length = std::stoi(it->second);

        // read exactly content_length characters from the remaining stream
        request.body.resize(content_length);
        request_stream.read(&request.body[0], content_length);
    }

    return request;
}
