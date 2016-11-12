#include <string>
#include <iostream>
#include <luna/luna.h>

using namespace luna;

void ex_log(log_level level, const std::string &message)
{
    std::cout << to_string(level) << " " << message << std::endl;
}

response hello_world(const request &req)
{
    return {"<h1>Hello, World!</h1>"};
}

int main(void)
{
    set_logger(ex_log);

    luna::server server{};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");

    while (server); //run forever, basically, or until the server decides to kill itself.
}