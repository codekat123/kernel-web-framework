#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../../include/server/TcpServer.hpp"
#include "../../include/router/Router.hpp"

TcpServer::TcpServer(
    int port,
    const Router& router
)
    : server_fd(-1),
      port(port),
      router(router),
      thread_pool(4),
      running(true)
{
}

void TcpServer::use(Middleware mw) {
    pipeline.use(mw);
}

void TcpServer::stop() {
    running = false;
    close(server_fd);
}

void TcpServer::setupSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd, 5) < 0) {
        throw std::runtime_error("Listen failed");
    }
}
