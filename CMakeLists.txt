cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(flow)
add_library(libflow
        messages.cpp
        parser.cpp
)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(test)
