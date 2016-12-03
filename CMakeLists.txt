cmake_minimum_required(VERSION 3.0)
project(wayward VERSION 2.0.0)
set(CMAKE_CXX_STANDARD 14)

include(ExternalProject)
include(CheckIncludeFiles)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

enable_testing()

check_include_files(asio.hpp ASIO_FOUND)
if (NOT ${ASIO_FOUND})
    message(FATAL "ASIO not found in include paths.")
endif()

ExternalProject_Add(libhttp_parser
    GIT_SUBMODULES external/http-parser
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/http-parser
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make package CFLAGS=-fPIC # Make suitable for inclusion in dynamic library
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

set(HTTPPARSER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/http-parser)
set(HTTPPARSER_LIBS ${CMAKE_SOURCE_DIR}/external/http-parser/libhttp_parser.a)


include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(wayward)
add_subdirectory(test)

