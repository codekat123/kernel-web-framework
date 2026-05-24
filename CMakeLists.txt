cmake_minimum_required(VERSION 3.10)

project(CppWebFramework)

set(CMAKE_CXX_STANDARD 17)


add_executable(
    server
    src/main.cpp
    src/server/TcpServer.cpp
    src/http/HttpRequest.cpp
	src/http/HttpResponse.cpp
)
