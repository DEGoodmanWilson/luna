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
    
        server.handle_request(request_method::GET, "/ohyeah", [](auto matches, auto params) -> response
            {
                return {"{\"koolade\": true}"};
            });
    
        server.handle_request(request_method::GET, "^/documents/(i[0-9a-f]{6})", [](auto matches, auto params) -> response
            {
                auto document_id = matches[1];
                return {"text/html", "<h1>Serving up document "+document_id+"</h1>"};
            });
    
        while (server); //idle while the server is running. Maybe not the best way? //TODO how to signal to server to die
    }

## TODO
  * Fix the way POSTDATA is handled so that we have a default handler, and a more sophisticated handler. Not super happy with the current flow.
  * Maybe make a CPM
  * fix coverage testing
  * docs docs docs
