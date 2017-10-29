cmake_minimum_required(VERSION 3.8)
project(back)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp csapp.cpp)
add_executable(back ${SOURCE_FILES})

target_link_libraries(back pthread)