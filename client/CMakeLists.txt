cmake_minimum_required(VERSION 3.8)
project(client)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES client.cpp csapp.cpp)

add_executable(client ${SOURCE_FILES})


target_link_libraries (client pthread)
