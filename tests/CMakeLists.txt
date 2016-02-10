cmake_minimum_required(VERSION 3.3)

#
#if (APPLE)
#    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
#    add_definitions(-D__GLIBCXX__)
#endif (APPLE)

include(ExternalProject)

set(GTEST_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/tests/gtest")
ExternalProject_Add(GTestExternal
        #        SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk
        #        SVN_REVISION -r746
        URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
        TIMEOUT 500
        PREFIX "${GTEST_PREFIX}"
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON)

set(LIBPREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")
set(LIBSUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(GTEST_LOCATION "${GTEST_PREFIX}/src/GTestExternal-build")
set(GTEST_INCLUDES "${GTEST_PREFIX}/src/GTestExternal/include")
set(GTEST_LIBRARY "${GTEST_LOCATION}/${LIBPREFIX}gtest${LIBSUFFIX}")
set(GTEST_MAINLIB "${GTEST_LOCATION}/${LIBPREFIX}gtest_main${LIBSUFFIX}")

add_library(GTest STATIC IMPORTED GLOBAL)
set_target_properties(GTest PROPERTIES
        IMPORTED_LOCATION "${GTEST_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDES}"
        IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

add_library(GTestMain STATIC IMPORTED GLOBAL)
set_target_properties(GTestMain PROPERTIES
        IMPORTED_LOCATION "${GTEST_MAINLIB}"
        IMPORTED_LINK_INTERFACE_LIBRARIES
        "${GTEST_LIBRARY};${CMAKE_THREAD_LIBS_INIT}")

add_dependencies(GTest GTestMain)


add_executable(luna_tests main.cpp)

add_dependencies(luna_tests GTestExternal luna)

target_include_directories(luna_tests SYSTEM
        PUBLIC ${GTEST_INCLUDES}
        PUBLIC ${LIB_LUNA_INCLUDE_DIRS}
        )

target_link_libraries(luna_tests GTestMain luna
        )