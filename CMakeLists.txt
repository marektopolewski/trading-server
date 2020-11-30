cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 17)

project(server)
set(SERVER_HEADER_FILES
        messages.cpp
        orderstore.cpp
        server.cpp
)
add_executable(server main.cpp ${SERVER_HEADER_FILES})

project(client)
set(CLIENT_HEADER_FILES
        messages.cpp
        orderstore.cpp
)
add_executable(client client.cpp ${CLIENT_HEADER_FILES})