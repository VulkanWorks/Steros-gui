cmake_minimum_required(VERSION 3.20)
project(steros C)

set(CMAKE_C_STANDARD 99)

include_directories(src)

add_library(steros src/app.h src/app.c src/widget.h src/button.h src/steros.h src/button.c)
add_executable(steros_test test_src/main.c)

target_link_libraries(steros
        glfw3
        vulkan
        pthread
        dl
        m)
target_link_libraries(steros_test steros)
