#pragma once

#include <string>
#include "../http/HttpResponse.hpp"
#include "../http/HttpRequest.hpp"

class StaticFileHandler {
public:
    StaticFileHandler(const std::string& root);
    HttpResponse serve(const std::string& url_path);

private:
    std::string root;
    std::string getMimeType(const std::string& file_path);
    bool isSafePath(const std::string& file_path);
};
