cmake_minimum_required(VERSION 2.8)
project(luna)

##### Use C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

##### External dependencies

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

if (BUILD_LUNA_COVERAGE)
    set(BUILD_LUNA_TESTS true) # need the tests if coverage is requested!

    if(NOT (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            AND (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"))
        message(FATAL_ERROR "Coveralls: Compiler ${CMAKE_C_COMPILER_ID} is not GNU gcc! Aborting... You can set this on the command line using CC=/usr/bin/gcc CXX=/usr/bin/g++ cmake <options> ..")
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(FATAL_ERROR "Coveralls: Code coverage results with an optimised (non-Debug) build may be misleading! Add -DCMAKE_BUILD_TYPE=Debug")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
endif()


##### luna
set(LUNA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
include_directories(SYSTEM ${LUNA_INCLUDE_DIRS})

set(LIB_LUNA_SOURCE_FILES luna/private/gcc49.cpp luna/types.h luna/server.cpp luna/server.h luna/private/server_impl.cpp luna/private/server_impl.h luna/config.cpp luna/config.h luna/private/gcc49.h)
add_library(luna ${LIB_LUNA_SOURCE_FILES} luna/luna.h)
include_directories(PRIVATE luna/private)
include_directories(SYSTEM PRIVATE luna)
target_link_libraries(luna microhttpd)

if (BUILD_LUNA_TESTS)
    message(STATUS "Building Luna with Tests")

    include(conanbuildinfo.cmake)
    CONAN_BASIC_SETUP()
    add_subdirectory(tests)
endif ()


if (BUILD_LUNA_EXAMPLES)
    message(STATUS "Building Luna with Examples")

    set(EXAMPLE1_SOURCE_FILES examples/example1.cpp)
    add_executable(example1 ${EXAMPLE1_SOURCE_FILES})
    target_link_libraries(example1 luna)

    set(EXAMPLE2_SOURCE_FILES examples/example2.cpp)
    add_executable(example2 ${EXAMPLE2_SOURCE_FILES})
    target_link_libraries(example2 luna)
endif ()
