cmake_minimum_required(VERSION 3.15)
project(city_manager C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -static")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

file(GLOB SOURCES src/*.c)
add_executable(city_manager ${SOURCES})
