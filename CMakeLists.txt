cmake_minimum_required(VERSION 2.8.4)
project(sysnet2p3)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(SOURCE_FILES client.c)
add_executable(sysnet2p3 ${SOURCE_FILES})