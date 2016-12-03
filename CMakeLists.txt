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

ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    CMAKE_ARGS -DBUILD_GTEST=1
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(googletest source_dir)
set(GOOGLETEST_INCLUDE_DIR ${source_dir}/googletest/include)
ExternalProject_Get_Property(googletest binary_dir)
set(GOOGLETEST_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_STATIC_LIBRARY_SUFFIX})
set(GOOGLETEST_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX})
set(GOOGLETEST_LIBRARY gtest)
set(GOOGLETEST_MAIN_LIBRARY gtest_main)
add_library(${GOOGLETEST_LIBRARY} UNKNOWN IMPORTED)
add_library(${GOOGLETEST_MAIN_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${GOOGLETEST_LIBRARY} PROPERTY IMPORTED_LOCATION ${GOOGLETEST_LIBRARY_PATH})
set_property(TARGET ${GOOGLETEST_MAIN_LIBRARY} PROPERTY IMPORTED_LOCATION ${GOOGLETEST_MAIN_LIBRARY_PATH})
add_dependencies(${GOOGLETEST_LIBRARY} googletest)
add_dependencies(${GOOGLETEST_MAIN_LIBRARY} ${GOOGLETEST_LIBRARY})


include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(wayward)
add_subdirectory(test)

