#pragma once

#include "../router/Router.hpp"


class TcpServer {
private:
    int server_fd;
    int port;
	Router router;

    void setupSocket();
    void handleClient(int client_socket);

public:
	TcpServer(
			int port,
			const Router& router
			);

    void start();
};
