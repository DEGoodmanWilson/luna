cmake_minimum_required(VERSION 2.8)

enable_testing()

add_executable(luna_tests
        main.cpp
        basic_functioning.cpp
        advanced_functioning.cpp
        crashers.cpp
        put.cpp
        patch.cpp
        delete.cpp
        server_options.cpp
        config.cpp
        types.cpp
        basic_auth.cpp
        tls.cpp
        safer_times.cpp
        file_service.cpp
        validation.cpp
    )

message(STATUS "CONAN_LIBS: ${CONAN_LIBS}")

target_link_libraries(luna_tests luna ${CONAN_LIBS})

add_test(luna ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/luna_tests)