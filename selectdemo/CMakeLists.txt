cmake_minimum_required(VERSION 3.8)
project(selectdemo)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp csapp.cpp )
add_executable(selectdemo ${SOURCE_FILES})

target_link_libraries(selectdemo pthread)




