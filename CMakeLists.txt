cmake_minimum_required(VERSION 3.18)
project(servPOS C)

set(CMAKE_C_STANDARD 99)

add_executable(servPOS main.c)

target_link_libraries(servPOS pthread)