cmake_minimum_required(VERSION 3.0)
project(wayward VERSION 2.0.0)
set(CMAKE_CXX_STANDARD 14)

include(ExternalProject)
include(CheckIncludeFiles)
check_include_files(asio/io_service.hpp ASIO_FOUND)
if (!ASIO_FOUND)
    message(FATAL "ASIO not found in include paths.")
endif(!ASIO_FOUND)

ExternalProject_Add(libhttp_parser
    GIT_SUBMODULES external/http-parser
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/http-parser
    PREFIX ${CMAKE_SOURCE_DIR}/external/http-parser
    CONFIGURE_COMMAND echo
    BUILD_COMMAND make libhttp_parser.o
    INSTALL_COMMAND echo
    BUILD_IN_SOURCE 1
    )

set(HTTPPARSER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/http-parser)
set(HTTPPARSER_LIBS ${CMAKE_SOURCE_DIR}/external/http-parser/libhttp_parser.o)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(wayward)
add_subdirectory(test)

