cmake_minimum_required(VERSION 2.8.4)
project(bbwriter)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(SOURCE_FILES bbWriter.c)
add_executable(bbwriter ${SOURCE_FILES})