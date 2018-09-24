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

# Examples from the documentation

set(INTRO_SOURCE_FILES intro.cpp)
add_executable(intro ${INTRO_SOURCE_FILES})
target_link_libraries(intro luna)

# Other examples

set(BASIC_WEBAPP_SOURCE_FILES basic_webapp.cpp)
add_executable(basic_webapp ${BASIC_WEBAPP_SOURCE_FILES})
target_link_libraries(basic_webapp luna)

set(TLS_SOURCE_FILES TLS.cpp)
add_executable(TLS ${TLS_SOURCE_FILES})
target_link_libraries(TLS luna)
