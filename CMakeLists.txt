cmake_minimum_required(VERSION 3.10.0)

project(MYTestProject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(SOURCES
    src/main.c
    src/parser.c
)

add_executable(runme ${SOURCES})

target_include_directories(runme PRIVATE src)