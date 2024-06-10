cmake_minimum_required(VERSION 3.28)
project(sandboxcpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(sandboxcpp main.cpp)

add_link_options(-pthread)
add_executable(tcpjump tcpjump.cpp)