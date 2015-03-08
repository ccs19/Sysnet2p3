cmake_minimum_required(VERSION 2.8.4)
project(bbserver)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

set(SOURCE_FILES bbserver.c)
add_executable(bbserver ${SOURCE_FILES})