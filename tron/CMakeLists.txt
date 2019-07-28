cmake_minimum_required(VERSION 2.8.12)
project(tron)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Curses REQUIRED)
set(CMAKE_C_FLAGS "-std=c99")

add_executable(tron tron.c)

target_link_libraries(tron curses)
