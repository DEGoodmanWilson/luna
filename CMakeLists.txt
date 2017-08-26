#
#       _
#   ___/__)
#  (, /      __   _
#    /   (_(_/ (_(_(_
#   (________________
#                    )
#
#  Luna
#  a web framework in modern C++
#
#  Copyright © 2016–2017 D.E. Goodman-Wilson
#

cmake_minimum_required(VERSION 2.8)

project(luna)

##### Use C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

##### External dependencies

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

##### Options

macro(luna_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})
    if(DEFINED ENV{${OPTION_NAME}})
        # Allow setting the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()
    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()
    message(STATUS "  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

message(STATUS "Luna CMake Options")
message(STATUS "=======================================================")
luna_option(BUILD_SHARED_LIBS   "Build the library as a dynamic object" OFF)
luna_option(BUILD_LUNA_TESTS    "Build the test suite"                  ON)
luna_option(BUILD_LUNA_COVERAGE "Generate test coverage information"    OFF)
luna_option(BUILD_LUNA_EXAMPLES "Build the example server"              ON)
message(STATUS "=======================================================")

set(LUNA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
include_directories(SYSTEM ${LUNA_INCLUDE_DIRS})
include_directories(SYSTEM PRIVATE luna)
include_directories(PRIVATE luna/private)
message(STATUS "Luna version:")
message(STATUS ${LUNA_VERSION})
add_definitions(-DLUNA_NAME=\"${CONAN_PACKAGE_NAME}\")
add_definitions(-DLUNA_VERSION=\"${CONAN_PACKAGE_VERSION}\")

if (BUILD_LUNA_COVERAGE)
    message(INFO "Will generate coverage information")
    set(BUILD_LUNA_TESTS true) # need the tests if coverage is requested!

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 --coverage")
endif()

if (BUILD_LUNA_TESTS)
    message(STATUS "Building Luna with Tests")

    add_definitions(-DLUNA_TESTING)
    add_definitions(-DLUNA_TESTING)

    add_subdirectory(tests)
endif ()


if (BUILD_LUNA_EXAMPLES)
    message(STATUS "Building Luna with Examples")

    add_subdirectory(examples)
endif ()

##### luna

set(LIB_LUNA_SOURCE_FILES
        luna/private/safer_times.cpp
        luna/types.cpp
        luna/types.h
        luna/server.cpp
        luna/server.h
        luna/private/server_impl.cpp
        luna/private/server_impl.h
        luna/config.cpp
        luna/config.h
        luna/private/safer_times.h
        luna/private/file_helpers.h
        luna/private/cacheable_response.cpp
        luna/private/cacheable_response.h
        luna/private/response_generator.cpp
        luna/private/response_generator.h
        )
add_library(luna ${LIB_LUNA_SOURCE_FILES} luna/luna.h)
target_link_libraries(luna ${CONAN_LIBS})