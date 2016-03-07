cmake_minimum_required(VERSION 3.2)
project(luna)

##### Use C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

##### External dependencies

## libmicrohttpd


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


##### luna
set(LUNA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
include_directories(SYSTEM ${LUNA_INCLUDE_DIRS})

set(LIB_LUNA_SOURCE_FILES luna/types.h luna/server.cpp luna/server.h luna/private/server_impl.cpp luna/private/server_impl.h luna/config.cpp luna/config.h)
add_library(luna ${LIB_LUNA_SOURCE_FILES} luna/luna.h)
target_include_directories(luna PRIVATE luna/private)
target_include_directories(luna PRIVATE luna)
target_link_libraries(luna microhttpd)



if (BUILD_LUNA_TESTS)
    enable_testing()
    include(conanbuildinfo.cmake)
    CONAN_BASIC_SETUP()
    add_subdirectory(tests)
endif ()


if (BUILD_LUNA_EXAMPLES)
    set(EXAMPLE1_SOURCE_FILES examples/example1.cpp)
    add_executable(example1 ${EXAMPLE1_SOURCE_FILES})
    target_link_libraries(example1 luna)

    set(EXAMPLE2_SOURCE_FILES examples/example2.cpp)
    add_executable(example2 ${EXAMPLE2_SOURCE_FILES})
    target_link_libraries(example2 luna)
endif ()
