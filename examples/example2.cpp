#include <string>
#include <sstream>
#include <luna/server.h>

using namespace luna;

response hello_world(const request &req)
{
    std::stringstream body;
    body << "<h1>Hello, World!</h1>\n<ul>\n";

    for(auto& kv : req.params)
    {
        body << "<li><b>" << kv.first << "</b> " << kv.second << "</li>\n";
    }

    body << "</ul>";
    return {body.str()};
}

int main(void)
{
    server server{server::port{8443}};

    server.handle_request(request_method::GET, "/hello_world", &hello_world);

    while (server); //run forever, basically, or until the server decides to kill itself.
}