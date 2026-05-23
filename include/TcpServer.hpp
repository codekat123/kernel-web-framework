#pragma once

#include <string>



class TcpServer {
	public:
		TcpServer(int port);
		
		void start();
	
	private:
		int server_fd;
		int port;
		
		void setupSocket();
		void handleClient(int client_socket);

};

