cmake_minimum_required(VERSION 2.8.4)
project(bbpeer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(SOURCE_FILES bbpeer.c bbWriter.c)
add_executable(bbwriter ${SOURCE_FILES})