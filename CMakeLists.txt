cmake_minimum_required(VERSION 3.6)
project(monitor)

file(GLOB CURSES_LIBRARIES libhere.a)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(monitor ${SOURCES})
# Udacity's workstation does not support C++17, fall back to C++11
set_property(TARGET monitor PROPERTY CXX_STANDARD 11)
target_link_libraries(monitor ${CURSES_LIBRARIES})
# TODO: Run -Werror in CI.
target_compile_options(monitor PUBLIC -Wall -Wextra -lcurses)
