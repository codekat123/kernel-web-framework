#include <iostream>
#include <csignal>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

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
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    finalizeRoutes();
    setupSocket();

    std::cout << "Server listening on port " << port << "...\n";

    while (running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(server_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (ready < 0) break;
        if (ready == 0) continue;

        sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_fd, (sockaddr*)&client_address, &client_len);

        if (client_socket < 0) {
            if (running) std::cerr << "Accept failed\n";
            continue;
        }

        thread_pool.enqueue([this, client_socket]() {
            handleClient(client_socket);
        });
    }

    std::cout << "Server shutting down...\n";
}
