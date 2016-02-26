#include <string>
#include <iostream>
#include <luna/server.h>

using namespace luna;

int main(void)
{
    server server{server::mime_type{"text/json"}, server::port{8443},
            //custom logger
                  [](const std::string mesg)
                      {
                          std::cout << mesg << std::endl;
                      }
    };

    server.handle_request(request_method::GET,
                          "/ohyeah",
                          [](auto matches, auto params) -> response
                              {
                                  return {"{\"koolade\": true}"};
                              });

    server.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto matches, auto params) -> response
                              {
                                  return {"hello"};
                              });

    server.handle_request(request_method::GET,
                          "^/documents/(i[0-9a-f]{6})",
                          [](auto matches, auto params) -> response
                              {
                                  auto document_id = matches[1];
                                  return {"text/html", "<h1>Serving up document " + document_id + "</h1>"};
                              });

    while (server); //idle while the server is running. Maybe not the best way? //TODO how to signal to server to die
}