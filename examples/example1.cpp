#include <string>
#include <iostream>
#include <luna/server.h>

using namespace luna;

int main(void)
{
    server server{server::port{8443}};

    server.handle_response(request_method::GET, "/ohyeah", [](std::vector<std::string> matches, query_params params, response& response) -> status_code
        {
            response = {"text/json", "{\"koolade\": true}"};
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