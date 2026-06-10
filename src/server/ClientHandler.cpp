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
        sizeof(buffer),
        0
    );

    if (bytes_received < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "recv error\n";
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

    std::string response_text = response.toString();

    send(
        client_socket,
        response_text.c_str(),
        response_text.length(),
        0
    );

    close(client_socket);
}
