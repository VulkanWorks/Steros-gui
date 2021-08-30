cmake_minimum_required(VERSION 3.20)
project(steros C)

set(CMAKE_C_STANDARD 99)

include_directories(src)

add_library(steros src/steros.h
        src/app.h src/app.c
#        src/ui/button.h src/ui/button.c
#        src/helper/option.h
        src/windowing/window.h src/windowing/window.c
        src/ntd/string.h
        )
add_executable(steros_test test_src/main.c)

target_link_libraries(steros
        xcb
        vulkan
        pthread
        dl
        m
        glfw3)
target_link_libraries(steros_test steros)
