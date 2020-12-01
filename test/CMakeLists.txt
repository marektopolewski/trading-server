cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

include(GoogleTest)

# Download GTest
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
)
if(result)
    message(FATAL_ERROR "Error, please download GTest manually.")
endif()

# Build GTest
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Error, please download GTest manually.")
endif()



add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL
)
add_executable(test
        financialinstrument.cpp
        orderstore.cpp
)
target_link_libraries(test libflow gmock_main)

gtest_discover_tests(test)