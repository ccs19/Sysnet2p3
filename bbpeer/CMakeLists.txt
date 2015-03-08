cmake_minimum_required(VERSION 2.8.4)
project(bbpeer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

set(SOURCE_FILES bbpeer.c bbwriter.c)
add_executable(bbpeer ${SOURCE_FILES})