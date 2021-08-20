cmake_minimum_required(VERSION 3.20)
project(steros C)

set(CMAKE_C_STANDARD 99)

include_directories(src)

add_library(steros src/app.h src/app.c src/app.c src/app.h src/UI/button.h src/UI/widget.h)
add_executable(steros_test test_src/main.c)

target_link_libraries(steros
        glfw3
        vulkan
        pthread
        dl
        m)
target_link_libraries(steros_test steros)
