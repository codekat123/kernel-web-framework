cmake_minimum_required(VERSION 3.10)

project(CppWebFramework)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(server ${SOURCES})

target_link_libraries(
    server
    sqlite3
)
