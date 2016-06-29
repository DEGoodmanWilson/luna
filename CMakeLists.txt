cmake_minimum_required(VERSION 2.8)

##### CPM
#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
set(CPM_MODULE_NAME luna)
set(CPM_LIB_TARGET_NAME ${CPM_MODULE_NAME})

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_TARGET_NAME))
    set(CPM_LIB_TARGET_NAME ${CPM_TARGET_NAME})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
    include(CPM)
else()
    set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
    find_package(Git)
    if(NOT GIT_FOUND)
        message(FATAL_ERROR "CPM requires Git.")
    endif()
    if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
        message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
        execute_process(
                COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
                RESULT_VARIABLE error_code
                OUTPUT_QUIET ERROR_QUIET)
        if(error_code)
            message(FATAL_ERROR "CPM failed to get the hash for HEAD")
        endif()
    endif()
    include(${CPM_DIR}/CPM.cmake)
endif()

# Include CPM modules or externals here (with CPM_AddModule).

CPM_InitModule(${CPM_MODULE_NAME})


##### Use C++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

##### External dependencies

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/coveralls/cmake)

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


#AND GENERATE_COVERAGE
#if(BUILD_LUNA_TESTS AND BUILD_LUNA_COVERAGE)
#    include(Coveralls)
#    coveralls_turn_on_coverage()
#endif()



##### luna
set(LIB_LUNA_SOURCE_FILES luna/types.h luna/server.cpp luna/server.h luna/private/server_impl.cpp luna/private/server_impl.h luna/config.cpp luna/config.h)
add_library(${CPM_LIB_TARGET_NAME} ${LIB_LUNA_SOURCE_FILES} luna/luna.h)
#target_include_directories(${CPM_LIB_TARGET_NAME} PRIVATE luna/private)
#target_include_directories(${CPM_LIB_TARGET_NAME} SYSTEM PRIVATE luna)
include_directories(PRIVATE luna/private)
include_directories(SYSTEM PRIVATE luna)
target_link_libraries(${CPM_LIB_TARGET_NAME} microhttpd)


#if (COVERALLS)
#    set(COVERAGE_SRCS ${LIB_LUNA_SOURCE_FILES})
#
#    # Create the coveralls target.
#    coveralls_setup(
#            "${COVERAGE_SRCS}" # The source files.
#            ON                 # If we should upload.
#            "${PROJECT_SOURCE_DIR}/coveralls/cmake")
#endif()

if (BUILD_LUNA_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()


if (BUILD_LUNA_EXAMPLES)
    set(EXAMPLE1_SOURCE_FILES examples/example1.cpp)
    add_executable(example1 ${EXAMPLE1_SOURCE_FILES})
    target_link_libraries(example1 ${CPM_LIB_TARGET_NAME})

    set(EXAMPLE2_SOURCE_FILES examples/example2.cpp)
    add_executable(example2 ${EXAMPLE2_SOURCE_FILES})
    target_link_libraries(example2 ${CPM_LIB_TARGET_NAME})
endif ()
