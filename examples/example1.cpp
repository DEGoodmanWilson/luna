#include <string>
#include <iostream>
#include <luna/luna.h>

using namespace luna;

void ex_error_log(log_level level, const std::string &message)
{
    std::cout << "[" << to_string(level) << "] " << message << std::endl;
}

void ex_access_log(const luna::request &request)
{
    std::cout << request.ip_address << ": " << luna::to_string(request.method) << " " << request.path << " " << request.http_version << " " << request.headers.at("user-agent") << std::endl;
}

response hello_world(const request &req)
{
    return {"<h1>Hello, World!</h1>"};
}

int main(void)
{
    server server{server::port{8443}};

    set_error_logger(ex_error_log);
    set_access_logger(ex_access_log);

    server.handle_request(request_method::GET, "/hello_world", &hello_world);

    server.await(); //run forever, basically, or until the server decides to kill itself.
}