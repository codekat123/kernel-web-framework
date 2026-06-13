#pragma once

#include <string>
#include <unordered_map>

class HttpResponse {
public:
    int status_code;
    std::string body;
    std::string content_type;
    std::unordered_map<std::string,std::string> headers;

    HttpResponse();

    void setHeader(const std::string& key, const std::string& value);
    std::string toString() const;
};
