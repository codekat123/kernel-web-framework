#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>

#include "../../include/server/TcpServer.hpp"
#include "../../include/http/HttpParser.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"

void TcpServer::handleClient(int client_socket) {
    char buffer[4096] = {0};

    ssize_t bytes_received = recv(
        client_socket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (bytes_received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            loop_.addFd(client_socket, [this](int fd) {
                loop_.removeFd(fd);
                thread_pool.enqueue([this, fd]() {
                    handleClient(fd);
                });
            });
            return;
        }
        close(client_socket);
        return;
    }

    if (bytes_received == 0) {
        close(client_socket);
        return;
    }

    HttpRequest request = HttpParser::parse(buffer);
    HttpResponse response;
    pipeline.execute(request, response);

    bool keep_alive = true;
    auto it = request.headers.find("Connection");
    if (it != request.headers.end() && it->second == "keep-alive") {
        keep_alive = false;
    }

    response.setHeader("Connection", keep_alive ? "keep-alive" : "close");

    std::string response_text = response.toString();
    send(
        client_socket,
        response_text.c_str(),
        response_text.length(),
        0
    );

    if (keep_alive) {
        loop_.addFd(client_socket, [this](int fd) {
            loop_.removeFd(fd);
            thread_pool.enqueue([this, fd]() {
                handleClient(fd);
            });
        });
    } else {
        close(client_socket);
    }
}
