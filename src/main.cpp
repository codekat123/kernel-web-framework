#include <iostream>

#include "../include/server/TcpServer.hpp"
#include "../include/http/HttpResponse.hpp"
#include "../include/router/Router.hpp"



HttpResponse homeHandler() {
    HttpResponse response;

    response.body = "Home Page";

    return response;
}

HttpResponse helloHandler() {
    HttpResponse response;

    response.body = "Hello From Router";

    return response;
}
int main() {
    try {
		Router router;
		router.addRoute("/",homeHandler);
		router.addRoute("/hello",helloHandler);
		
		TcpServer server(8080,router);
		server.start();

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}
