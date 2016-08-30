#include <string>
#include <iostream>
#include <luna/luna.h>

using namespace luna;

void log(log_level level, const std::string &message)
{
    std::cout << to_string(level) << " " << message << std::endl;
}

response hello_world(const request &req)
{
    return {"<h1>Hello, World!</h1>"};
}

int main(void)
{
    server server{server::port{8443}};

    set_logger(log);

    server.handle_request(request_method::GET, "/hello_world", &hello_world);

    while (server); //run forever, basically, or until the server decides to kill itself.
}