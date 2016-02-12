# luna

An embedded HTTP server in idiomatic C++

-or-

A C++ wrapper for libmicrohttpd

#include <string>
#include <iostream>
#include <luna/server.h>

    using namespace luna;
    
    int main(void)
    {
        server server{server::mime_type{"text/json"}, server::port{8443}, [](uint16_t error_code,
                                                                             request_method method,
                                                                             const std::string &path) -> response
            {
                return {"<h1>Oh, that's a problem</h1>"};
            }
        };
    
        server.handle_response(request_method::GET, "/ohyeah", [](std::vector<std::string> matches, query_params params, response& response) -> status_code
            {
                response = {"{\"koolade\": true}"};
                return 200;
            });
    
        server.handle_response(request_method::GET, "^/documents/(i[0-9a-f]{6})", [](std::vector<std::string> matches, query_params params, response& response) -> status_code
            {
                auto document_id = matches[1];
                response = {"text/html", "<h1>Serving up document "+document_id+"</h1>"};
                return 200;
            });
    
        server.start();
    
        while (1); //run until you get killed. Not the best way.
    }