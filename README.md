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
        server server{server::mime_type{"text/json"}, server::port{8443}}
    
        server.handle_response(request_method::GET, "/ohyeah", [](std::vector<std::string> matches, query_params params) -> response
            {
                return {"{\"koolade\": true}"};
            });
    
        server.handle_response(request_method::GET, "^/documents/(i[0-9a-f]{6})", [](std::vector<std::string> matches, query_params params) -> response
            {
                auto document_id = matches[1];
                return {"text/html", "<h1>Serving up document "+document_id+"</h1>"};
            });
    
        while (server); //idle while the server is running. Maybe not the best way? //TODO how to signal to server to die
    }

## TODO
  * Maybe make a CPM
  * fix coverage testing
  * docs docs docs
