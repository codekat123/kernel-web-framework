#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include "../../include/static/StaticFileHandler.hpp"

StaticFileHandler::StaticFileHandler(const std::string& root)
    : root(root) {}

std::string StaticFileHandler::getMimeType(const std::string& path) {
    size_t dot = path.rfind('.');

    if (dot == std::string::npos) {
        return "application/octet-stream";
    }

    std::string ext = path.substr(dot);

    if (ext == ".html") return "text/html";
    if (ext == ".css")  return "text/css";
    if (ext == ".js")   return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg")  return "image/jpeg";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".txt")  return "text/plain";

    return "application/octet-stream";
}

bool StaticFileHandler::isSafePath(const std::string& file_path) {
    std::filesystem::path root_path =
        std::filesystem::canonical(
            std::filesystem::current_path() / root
        );

    std::filesystem::path requested =
        std::filesystem::weakly_canonical(
            std::filesystem::current_path() / file_path
        );

    std::string root_str = root_path.string();
    std::string req_str  = requested.string();

    return req_str.substr(0, root_str.size()) == root_str;
}

HttpResponse StaticFileHandler::serve(const std::string& url_path) {
    HttpResponse response;

    std::string file_path = root + url_path;

    if (!isSafePath(file_path)) {
        response.status_code = 403;
        response.body = "403 Forbidden";
        return response;
    }

    std::ifstream file(file_path);

    if (!file.is_open()) {
        response.status_code = 404;
        response.body = "404 Not Found";
        return response;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    response.body = buffer.str();
    response.content_type = getMimeType(file_path);
    response.status_code = 200;

    return response;
}
