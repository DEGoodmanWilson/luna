cmake_minimum_required(VERSION 3.3)
project(luna)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

##### Options
option(BUILD_SHARED_LIBS "Build the library as a dynamic object" OFF)
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_EXAMPLES "Build the example server" ON)

# luna
set(LIB_LUNA_SOURCE_FILES server.cpp luna/server.h server_impl.cpp server_impl.h)
add_library(luna ${LIB_LUNA_SOURCE_FILES})
target_include_directories(luna PRIVATE luna)
target_link_libraries(luna microhttpd)

set(LIB_LUNA_INCLUDE_DIRS ${CMAKE_SOURCE_DIR})


if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()


if (BUILD_EXAMPLES)
    set(EXAMPLE_SOURCE_FILES examples/example1.cpp)
    add_executable(example1 ${EXAMPLE_SOURCE_FILES})
    target_include_directories(example1 SYSTEM PUBLIC ${LIB_LUNA_INCLUDE_DIRS})
    target_link_libraries(example1 luna)
endif ()