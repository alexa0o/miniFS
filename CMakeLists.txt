cmake_minimum_required(VERSION 3.10)
project(miniFS C)

set(CMAKE_C_STANDARD 99)

add_executable(miniFS main.c fs.c utils.c)