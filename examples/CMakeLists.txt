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

# Examples from the documentation

set(INTRO_1_SOURCE_FILES intro_1.cpp)
add_executable(intro_1 ${INTRO_1_SOURCE_FILES})
target_link_libraries(intro_1 luna)

# Other examples

set(BASIC_WEBAPP_SOURCE_FILES basic_webapp.cpp project_template/main.cpp project_template/logger.cpp project_template/logger.h)
add_executable(basic_webapp ${BASIC_WEBAPP_SOURCE_FILES})
target_link_libraries(basic_webapp luna)

set(TLS_SOURCE_FILES TLS.cpp)
add_executable(TLS ${TLS_SOURCE_FILES})
target_link_libraries(TLS luna)