#include <iostream>
#include <luna/luna.h>
#include <nlohmann/json.hpp>
#include "logger.h"
#include <string>
#include <vector>
#include <experimental/array>

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

    std::vector<std::string> container;

    // create a server
    server server;
    // add endpoints

    // API example, served from /api
    auto api = server.create_router("/api");
    api->handle_request(request_method::GET, "/endpoint",
                       [&](auto request) -> response
                       {
                            nlohmann::json retval;
                            for(decltype(container)::iterator itr; itr != std::end(container); itr++)
                                retval[std::distance(std::begin(container), itr)] = *itr;
                            
                           return retval.dump();
                       });

    auto post = server.create_router("/api/");
    post->handle_request(request_method::POST, "/post",
                        [&](auto request) -> response
                        {

                            nlohmann::json retval;

                            container.emplace_back(std::move(request.params.at("text")));

                            retval[std::to_string(container.size())] = request.params.at("text");

                            return retval.dump();
                        });

    // File serving example; serve files from the assets folder on /
    // index pages

    auto router = server.create_router("/");

    auto routers = std::experimental::make_array("/", "/about", "/contact");

    for(auto&& r : routers)
        router->handle_request(request_method::GET, 
                            r, 
                            [](auto request) -> response { 
                                response res = response::from_file("./assets/index.html");
                            return res;
                        });

    router->handle_request(request_method::GET, "/webpack/index.js",[] (auto req) -> response {
            response res = response::from_file("./assets/webpack/index.js");
            res.content_type = "text/javascript";
            return res;
    });

    server.start(port);

    return 0;
}
