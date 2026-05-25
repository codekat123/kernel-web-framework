#include <sstream>

#include "../../include/http/HttpResponse.hpp"




std::string getStatusText(int status_code) {
	switch (status_code) {
		case 200:
			return "OK";
		case 404:
			return "Not Found";
		case 500:
			return "Internal Server Error";
		default:
			return "Unknown";

	}
}

HttpResponse::HttpResponse()
    : status_code(200),
      content_type("text/plain") {}

std::string HttpResponse::toString() const {
    std::ostringstream response;

    response << "HTTP/1.1 "
             << status_code
             << " "
			 << getStatusText(status_code)
			 << "\r\n";

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
