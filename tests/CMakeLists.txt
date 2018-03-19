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


add_executable(${PROJECT_NAME}_tests
        ${LIB_LUNA_SOURCE_FILES}
        main.cpp
        basic_functioning.cpp
        advanced_functioning.cpp
        crashers.cpp
        put.cpp
        patch.cpp
        delete.cpp
        config.cpp
        types.cpp
        basic_auth.cpp
        tls.cpp
        safer_times.cpp
        file_service.cpp
        validation.cpp
        middleware.cpp
        logging.cpp
        options.cpp
        server_impl.cpp
        caching.cpp
        server_options.cpp
        headers.cpp
        )

FOREACH(LETTER ${CONAN_LIBS})
    MESSAGE(STATUS "${LETTER}")
ENDFOREACH()


target_link_libraries(${PROJECT_NAME}_tests ${CONAN_LIBS})