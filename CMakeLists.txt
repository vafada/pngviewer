cmake_minimum_required(VERSION 3.31)
project(pngviewer C)

set(CMAKE_C_STANDARD 17)

if (${APPLE})
    message ("MAC OS X")
    include_directories(/opt/homebrew/Cellar/raylib/5.5/include/)
endif ()

add_executable(pngviewer main.c)

target_link_libraries(pngviewer raylib)
target_link_libraries(pngviewer z)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/basn6a08.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/basn3p04.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fish.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pp0n2c16.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bee.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/nature.png DESTINATION ${CMAKE_BINARY_DIR})