#include <iostream>
#include <luna/luna.h>
#include <json.hpp>
#include "logger.h"

int main()
{
    // set up the loggers
    luna::set_access_logger(access_logger);
    luna::set_error_logger(error_logger);

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
            error_logger(luna::log_level::FATAL, "Invalid port specified in env $PORT.");
            return 1;
        }
        catch (const std::out_of_range &e)
        {
            error_logger(luna::log_level::FATAL, "Port specified in env $PORT is too large.");
            return 1;
        }
    }

    // add endpoints

    // API example, served from /api
    luna::router api{"/api"};
    api.handle_request(request_method::GET, "/endpoint",
                       [](auto request) -> response
                       {
                           return {"{\"made_it\": true}"};
                       });

    // File serving example; serve files from the assets folder on /
    luna::router static_assets{"/"};
    static_assets.serve_files("/", "assets");


    // create a server
    luna::server server;
    server.add_router(api);
    server.add_router(static_assets);

    server.start(port);

    return 0;
}