#include <string>
#include <luna/server.h>

using namespace luna;

response hello_world(const request &req)
{
    return {"<h1>Hello, World!</h1>"};
}

int main(void)
{
    server server{server::port{8443}};

    server.handle_request(request_method::GET, "/hello_world", &hello_world);

    while (server); //run forever, basically, or until the server decides to kill itself.
}