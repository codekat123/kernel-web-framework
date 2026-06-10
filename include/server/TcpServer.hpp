#pragma once

#include <atomic>

#include "../router/Router.hpp"
#include "../middleware/MiddlewarePipeline.hpp" 
#include "../threading/ThreadPool.hpp"
#include "EventLoop.hpp"


class TcpServer {
private:
    int server_fd;
    int port;
    Router router;
    MiddlewarePipeline pipeline;
    ThreadPool thread_pool;
    std::atomic<bool> running;
    EventLoop loop_;

    void setupSocket();
    void finalizeRoutes();
    void acceptClients();
    void handleClient(int client_socket);

public:
    TcpServer(int port, const Router& router);
    void use(Middleware mw);  
    void start();
    void stop();
};
