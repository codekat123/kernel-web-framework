#pragma once

#include "../router/Router.hpp"
#include "../middleware/MiddlewarePipeline.hpp" 
#include "../threading/ThreadPool.hpp"

class TcpServer {
private:
    int server_fd;
    int port;
    Router router;
    MiddlewarePipeline pipeline;
    ThreadPool thread_pool;

    void setupSocket();
    void handleClient(int client_socket);

public:
    TcpServer(int port, const Router& router);
    void use(Middleware mw);  
    void start();
};
