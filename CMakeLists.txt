cmake_minimum_required(VERSION 3.10.0)

project(MYTestProject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(SOURCES
    src/main.c
    src/parser.c
    src/http.c
    src/sock_utils.c
)

find_package(OpenSSL REQUIRED)

add_executable(runme ${SOURCES})

target_link_libraries(runme PRIVATE OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(runme PRIVATE src)