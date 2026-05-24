#include <sstream>

#include "../../include/http/HttpResponse.hpp"

HttpResponse::HttpResponse()
    : status_code(200),
      content_type("text/plain") {}

std::string HttpResponse::toString() const {
    std::ostringstream response;

    response << "HTTP/1.1 "
             << status_code
             << " OK\r\n";

    response << "Content-Type: "
             << content_type
             << "\r\n";

    response << "Content-Length: "
             << body.length()
             << "\r\n";

    response << "\r\n";

    response << body;

    return response.str();
}
