cmake_minimum_required(VERSION 3.2)
project(luna)

##### CPM
include(cpm.cmake)


##### Use C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

##### Options
option(BUILD_SHARED_LIBS "Build the library as a dynamic object" OFF)
option(BUILD_LUNA_TESTS "Build the test suite" ON)
option(BUILD_LUNA_COVERAGE "Generate test coverage information" OFF)
option(BUILD_LUNA_EXAMPLES "Build the example server" ON)



#AND GENERATE_COVERAGE)
if(BUILD_LUNA_TESTS AND BUILD_LUNA_COVERAGE)
    set(CMAKE_BUILD_TYPE COVERAGE CACHE INTERNAL "Coverage enabled build")
    message(STATUS "Enabling gcov support")
    set(CMAKE_CXX_FLAGS_COVERAGE
            "-g -O0 -fprofile-arcs -ftest-coverage"
            CACHE STRING "Flags used by the C++ compiler during coverage builds."
            FORCE)
    set(CMAKE_C_FLAGS_COVERAGE
            "-g -O0 -fprofile-arcs -ftest-coverage"
            CACHE STRING "Flags used by the C compiler during coverage builds."
            FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
            "-fprofile-arcs -ftest-coverage"
            CACHE STRING "Flags used for linking binaries during coverage builds."
            FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
            "-fprofile-arcs -ftest-coverage"
            CACHE STRING "Flags used by the shared libraries linker during coverage builds."
            FORCE)
    mark_as_advanced(
            CMAKE_CXX_FLAGS_COVERAGE
            CMAKE_C_FLAGS_COVERAGE
            CMAKE_EXE_LINKER_FLAGS_COVERAGE
            CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
endif()




# TODO script to find libmicrohttpd

# luna
set(LIB_LUNA_SOURCE_FILES luna/types.h luna/server.cpp luna/server.h luna/private/server_impl.cpp luna/private/server_impl.h)
add_library(luna ${LIB_LUNA_SOURCE_FILES} luna/luna.h)
target_include_directories(luna PRIVATE luna/private)
target_include_directories(luna SYSTEM PRIVATE luna)
target_link_libraries(luna microhttpd)



if (BUILD_LUNA_TESTS)
    enable_testing()
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