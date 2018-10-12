#
#       _
#   ___/_)
#  (, /      ,_   _
#    /   (_(_/ (_(_(_
#  CX________________
#                    )
#
#  Luna
#  A web application and API framework in modern C++
#
#  Copyright © 2016–2018 D.E. Goodman-Wilson
#

cmake_minimum_required(VERSION 3.6)

project(luna)

##### Use C++14 or 17 when available
include(CheckCXXCompilerFlag)

# Check for standard to use. This is going to depend on compiler and version, because I couldn't get check_cxx_compile_flag to work
message(STATUS ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})

check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
    # Have -std=c++17, use it
    message(STATUS "Luna using C++17")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    check_cxx_compiler_flag(-std=c++14 HAVE_FLAG_STD_CXX14)
    if(HAVE_FLAG_STD_CXX14)
        # Have -std=c++14, use it
        message(STATUS "Luna using C++14")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Luna requires at least C++14")
    endif()
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
luna_option(BUILD_LUNA_TESTS    "Build the test suite"                  OFF)
luna_option(BUILD_LUNA_COVERAGE "Generate test coverage information"    OFF)
luna_option(BUILD_LUNA_EXAMPLES "Build the example server"              OFF)
message(STATUS "=======================================================")

set(LUNA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
include_directories(SYSTEM ${LUNA_INCLUDE_DIRS})
include_directories(SYSTEM PRIVATE luna)
include_directories(PRIVATE luna/private)
message(STATUS "Luna version: " ${CONAN_PACKAGE_VERSION})
add_definitions(-DLUNA_NAME=\"${CONAN_PACKAGE_NAME}\")
add_definitions(-DLUNA_VERSION=\"${CONAN_PACKAGE_VERSION}\")

if (BUILD_LUNA_COVERAGE)
    message(INFO "Will generate coverage information")
    set(BUILD_LUNA_TESTS true) # need the tests if coverage is requested!

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 --coverage")
endif()



##### luna
if (BUILD_LUNA_TESTS)
    add_definitions(-DLUNA_TESTING)
    add_definitions(-DSTATIC_ASSET_PATH="${PROJECT_SOURCE_DIR}")
endif ()

set(LIB_LUNA_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/luna/private/safer_times.cpp
        ${PROJECT_SOURCE_DIR}/luna/types.cpp
        ${PROJECT_SOURCE_DIR}/luna/types.h
        ${PROJECT_SOURCE_DIR}/luna/server.cpp
        ${PROJECT_SOURCE_DIR}/luna/server.h
        ${PROJECT_SOURCE_DIR}/luna/private/server_impl.cpp
        ${PROJECT_SOURCE_DIR}/luna/private/server_impl.h
        ${PROJECT_SOURCE_DIR}/luna/config.cpp
        ${PROJECT_SOURCE_DIR}/luna/config.h
        ${PROJECT_SOURCE_DIR}/luna/private/safer_times.h
        ${PROJECT_SOURCE_DIR}/luna/private/file_helpers.h
        ${PROJECT_SOURCE_DIR}/luna/private/cacheable_response.cpp
        ${PROJECT_SOURCE_DIR}/luna/private/cacheable_response.h
        ${PROJECT_SOURCE_DIR}/luna/private/response_renderer.cpp
        ${PROJECT_SOURCE_DIR}/luna/private/response_renderer.h
        ${PROJECT_SOURCE_DIR}/luna/router.cpp
        ${PROJECT_SOURCE_DIR}/luna/router.h
        ${PROJECT_SOURCE_DIR}/luna/optional.hpp
        ${PROJECT_SOURCE_DIR}/luna/private/router_impl.h
        ${PROJECT_SOURCE_DIR}/luna/private/router_impl.cpp
        )

add_library(${PROJECT_NAME} ${LIB_LUNA_SOURCE_FILES} luna/luna.h)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})



##### tests

if (BUILD_LUNA_TESTS)
    message(STATUS "Building Luna with Tests")

    enable_testing()
    add_subdirectory(tests)
    add_test(ALL_TESTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_tests)
endif ()



##### examples

if (BUILD_LUNA_EXAMPLES)
    message(STATUS "Building Luna with Examples")

    add_subdirectory(examples)
endif ()
