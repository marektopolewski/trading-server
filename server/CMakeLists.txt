cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
add_executable(server
        financialintrument.cpp
        main.cpp
        orderstore.cpp
        server.cpp
)
target_link_libraries(server libflow)
add_library(libserver
        financialintrument.cpp
        orderstore.cpp
)
