cmake_minimum_required(VERSION 2.8.12)
project(snake)

find_package(Curses REQUIRED)
set(CMAKE_C_FLAGS "-std=c99")

add_executable(snake snake.c)

target_link_libraries(snake curses)
