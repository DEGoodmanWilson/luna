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


set(BASIC_WEBAPP_SOURCE_FILES basic_webapp.cpp)
add_executable(basic_webapp ${BASIC_WEBAPP_SOURCE_FILES})
target_link_libraries(basic_webapp luna)

set(TLS_SOURCE_FILES TLS.cpp)
add_executable(TLS ${TLS_SOURCE_FILES})
target_link_libraries(TLS luna)