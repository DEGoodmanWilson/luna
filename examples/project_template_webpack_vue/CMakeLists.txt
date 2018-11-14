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

project(awesomesauce)

##### Use C++14 or 17 when available

include(CheckCXXCompilerFlag)



check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
    # Have -std=c++17, use it
    message(STATUS "Luna using C++17")
	## because I simply cannot get `set(CMAKE_CXX_STANDARD 17) to work
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    check_cxx_compiler_flag(-std=c++14 HAVE_FLAG_STD_CXX14)
    if(HAVE_FLAG_STD_CXX14)
        # Have -std=c++14, use it
        message(STATUS "Luna using C++14")
		## because I simply cannot get `set(CMAKE_CXX_STANDARD 14) to work
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Luna requires at least C++14")
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	## because I simply cannot get `set(CMAKE_CXX_STANDARD 14) to work
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

set(SOURCE_FILES
        logger.h
        logger.cpp
        main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

# Tests
add_subdirectory(tests)