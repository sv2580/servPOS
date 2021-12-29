cmake_minimum_required(VERSION 3.18)
project(sem C)

set(CMAKE_C_STANDARD 99)

add_executable(sem main.c client.h client.c server.h server.c)

target_link_libraries(sem pthred)