cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 17)

project(flow)

set(SERVER_HEADER_FILES
        financialintrument.cpp
        messages.cpp
        orderstore.cpp
        server.cpp
)
add_executable(server main.cpp ${SERVER_HEADER_FILES})

set(CLIENT_HEADER_FILES
        messages.cpp
)
add_executable(client client.cpp ${CLIENT_HEADER_FILES})


add_library(libflow
    client.cpp
    financialintrument.cpp
    messages.cpp
    orderstore.cpp
    server.cpp
)

add_subdirectory(test)
