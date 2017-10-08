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
#  Copyright © 2016–2017 D.E. Goodman-Wilson
#

cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME awesomesauce)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)

include(conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

set(SOURCE_FILES
        main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

# Tests
add_subdirectory(tests)