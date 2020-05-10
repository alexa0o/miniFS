cmake_minimum_required(VERSION 3.10)
project(miniFS C)

set(CMAKE_C_STANDARD 99)

include_directories(fs utils network)

add_executable(miniFS main.c fs/fs.c utils/utils.c)

add_executable(serverFS fs/fs.c utils/utils.c network/server.c)

add_executable(clientFS network/client.c)