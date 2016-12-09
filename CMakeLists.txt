cmake_minimum_required(VERSION 3.0)
project(wayward VERSION 2.0.0)
set(CMAKE_CXX_STANDARD 14)

include(ExternalProject)
include(CheckIncludeFileCXX)
include(FindBoost)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

enable_testing()

check_include_file_cxx("asio.hpp" ASIO_FOUND)
find_package(Boost)

# Prefer ASIO over Boost.ASIO
if (${ASIO_FOUND})
    set(ASIO_INCLUDE_DIR "" CACHE STRING "")
    set(ASIO_LIBRARY DIR "" CACHE STRING "")
    set(ASIO_LIBRARIES "" CACHE STRING "")
    set(ASIO_FROM_BOOST "0" CACHE STRING "")
elseif (${Boost_FOUND})
    set(ASIO_INCLUDE_DIR "${Boost_INCLUDE_DIRS}" CACHE STRING "")
    set(ASIO_LIBRARY_DIR "${Boost_LIBRARY_DIRS}" CACHE STRING "")
    set(ASIO_LIBRARIES "${Boost_ASIO_LIBRARY}" CACHE STRING "")
    set(ASIO_FROM_BOOST "1" CACHE STRING "")
else()
    message(FATAL_ERROR "ASIO or Boost.ASIO not found in include paths. Try setting BOOST_ROOT, or install ASIO headers.")
endif()

add_library(http_parser ${CMAKE_SOURCE_DIR}/external/http-parser/http_parser.c)

set(HTTPPARSER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/http-parser)
set(HTTPPARSER_LIBS http_parser)

ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    CMAKE_ARGS -DBUILD_GTEST=ON -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
    INSTALL_DIR ${CMAKE_BINARY_DIR}
    LOG_INSTALL 1
)
set(GOOGLETEST_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(GOOGLETEST_LIBRARY_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_STATIC_LIBRARY_SUFFIX})
set(GOOGLETEST_MAIN_LIBRARY_PATH ${CMAKE_BINARY_DIR}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX})
add_library(gtest UNKNOWN IMPORTED)
add_library(gtest_main UNKNOWN IMPORTED)
set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${GOOGLETEST_LIBRARY_PATH})
set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${GOOGLETEST_MAIN_LIBRARY_PATH})
add_dependencies(gtest googletest)
add_dependencies(gtest_main gtest)

include_directories(${ASIO_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})
link_directories(${ASIO_LIBRARY_DIR})

configure_file(
    wayward/config.h.in
    ${CMAKE_BINARY_DIR}/config.h
)

add_subdirectory(wayward)
add_subdirectory(test)

