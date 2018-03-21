#include <iostream>
#include <luna/luna.h>
#include <nlohmann/json.hpp>
#include "logger.h"

using namespace luna;

int main()
{
    // set up the loggers
    set_access_logger(access_logger);
    set_error_logger(error_logger);

    // determine which port to run on, default to 8080
    auto port = 8080;
    if (auto port_str = std::getenv("PORT"))
    {
        try
        {
            port = std::atoi(port_str);
        }
        catch (const std::invalid_argument &e)
        {
            error_logger(log_level::FATAL, "Invalid port specified in env $PORT.");
            return 1;
        }
        catch (const std::out_of_range &e)
        {
            error_logger(log_level::FATAL, "Port specified in env $PORT is too large.");
            return 1;
        }
    }

    // create a server
    server server;
    // add endpoints

    // API example, served from /api
    auto api = server.create_router("/api");
    api->handle_request(request_method::GET, "/endpoint",
                       [](auto request) -> response
                       {
                           nlohmann::json retval;
                           retval["made_it"] = true;
                           return retval.dump();
                       });

    // File serving example; serve files from the assets folder on /
    auto static_assets = server.create_router("/");
    static_assets->serve_files("/", "assets");


    server.start(port);

    return 0;
}
