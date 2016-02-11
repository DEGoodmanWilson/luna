//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <string>
#include <regex>
#include <memory>
#include <map>
#include <functional>

namespace luna
{

class server
{
public:

    // configuration parameters
    MAKE_UINT16_T_LIKE(port);

    server(uint16_t port);

    ~server();

    //TODO need a method to set a defailut response
    using endpoint_handler_cb = std::function<status_code(std::vector<std::string> matches,
                                                          query_params params,
                                                          response &body_out)>;

    using error_handler_cb = std::function<response(uint16_t error_code,
                                                    request_method method,
                                                    const std::string &path)>;


    template<typename T>
    void handle_response(request_method method, T path, endpoint_handler_cb callback)
    {
        handle_response(method, std::regex{std::forward<T>(path)}, callback);
    }

    template
    void handle_response(request_method method, const std::regex &path, endpoint_handler_cb callback);

    template
    void handle_response(request_method method, std::regex &&path, endpoint_handler_cb callback);


    void set_error_handler(error_handler_cb handler);

    bool start();


private:
    class server_impl;

    std::unique_ptr<server_impl> impl_;
};

} //namespace luna
