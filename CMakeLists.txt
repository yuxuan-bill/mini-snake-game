cmake_minimum_required(VERSION 3.12)
project(firstc C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-lncurses")

add_executable(firstc Snake.c)