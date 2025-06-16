cmake_minimum_required(VERSION 3.31)
project(pngviewer C)

set(CMAKE_C_STANDARD 17)

add_executable(pngviewer main.c)

target_link_libraries(pngviewer raylib)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/basn6a08.png DESTINATION ${CMAKE_BINARY_DIR})