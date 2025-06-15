cmake_minimum_required(VERSION 3.31)
project(pngviewer C)

set(CMAKE_C_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(pngviewer main.c)

target_link_libraries(pngviewer SDL2)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/basn6a08.png DESTINATION ${CMAKE_BINARY_DIR})