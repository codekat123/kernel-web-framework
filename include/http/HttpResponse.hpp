#pragma once

#include <string>

class HttpResponse {
public:
    int status_code;
    std::string body;
    std::string content_type;

    HttpResponse();

    std::string toString() const;
};
