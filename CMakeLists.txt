cmake_minimum_required(VERSION 3.10.0)

project(MYTestProject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(SOURCES
    src/main.c
    src/parser.c
    src/http.c
    src/sock_utils.c
    src/utils.c
)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find liburing
find_library(LIBURING_LIBRARY 
    NAMES uring
    PATHS /usr/lib /usr/local/lib
)

if(NOT LIBURING_LIBRARY)
    message(FATAL_ERROR "liburing not found. Please install liburing-dev")
endif()

# Add include directories for liburing if needed
find_path(LIBURING_INCLUDE_DIR 
    NAMES liburing.h
    PATHS /usr/include /usr/local/include
)

add_executable(runme ${SOURCES})

# Link libraries
target_link_libraries(runme PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${LIBURING_LIBRARY}
)

# Include directories
target_include_directories(runme PRIVATE 
    src
    ${LIBURING_INCLUDE_DIR}
)