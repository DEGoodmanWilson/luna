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

target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME} ${CONAN_LIBS})