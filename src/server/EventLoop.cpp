#include <iostream>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../../include/server/TcpServer.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"

static TcpServer* global_server_instance = nullptr;

void handleSignal(int signal) {
    if (global_server_instance) {
        global_server_instance->stop();
    }
}

void TcpServer::finalizeRoutes() {
    pipeline.use([this](
        HttpRequest& req,
        HttpResponse& res,
        std::function<void()> next
    ) {
        res = this->router.route(req.path, req);
    });
}

void TcpServer::start() {
    global_server_instance = this;
    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    finalizeRoutes();
    setupSocket();

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    loop_.addFd(server_fd, [this](int fd) {
        acceptClients();
    });

    loop_.setServerFd(server_fd);

    std::cout << "Server listening on port " << port << "...\n";

    loop_.run();

    std::cout << "Server shutting down...\n";
}

void TcpServer::stop() {
    running = false;
    loop_.stop();
    close(server_fd);
    server_fd = -1;
}

void TcpServer::acceptClients() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t   client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "accept error\n";
            break;
        }

        loop_.addFd(client_fd, [this](int fd) {
            loop_.removeFd(fd);

            thread_pool.enqueue([this, fd]() {
                handleClient(fd);
            });
        });
    }
}
