#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <csignal>
#include <atomic>


#include "../../include/server/TcpServer.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/router/Router.hpp"
#include "../../include/http/HttpParser.hpp"
#include "../../include/threading/ThreadPool.hpp"


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

static TcpServer* global_server_instance = nullptr;

void handleSignal(int singal) {
    if (global_server_instance) {
        global_server_instance->stop();
    }
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
void TcpServer::finalizeRoutes() {
    pipeline.use([this](
        HttpRequest& req,
        HttpResponse& res,
        std::function<void()> next
    ) {
        res = this->router.route(req.path, req);
    });
}

void TcpServer::handleClient(int client_socket) {
    char buffer[4096] = {0};

    ssize_t bytes_received = recv(
        client_socket,
        buffer,
        sizeof(buffer),
        0
    );

    HttpRequest request;

    if (bytes_received > 0) {
        std::cout << "==== Incoming Request ====\n";
        std::cout << buffer << "\n";
        request = HttpParser::parse(buffer);
    }

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

        if (ready < 0) {
            break;
        }
        if (ready == 0) {
            continue;
        }

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
