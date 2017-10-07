//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <luna/config.h>
#include <luna/optional.hpp>
#include <regex>

namespace luna
{

class router
{
public:
    MAKE_STRING_LIKE(mime_type);

    router(std::string route_base = "")
    {
        initialize_(route_base);
    }

    template<typename ...Os>
    router(Os &&...os)
    {
        initialize_("");
        set_options_(LUNA_FWD(os)...);
    }

    template<typename ...Os>
    router(std::string route_base, Os &&...os)
    {
        initialize_(route_base);
        set_options_(LUNA_FWD(os)...);
    }

    router(const router &r);

    router(router &&r);

    using endpoint_handler_cb = std::function<response(const request &req)>;

    void handle_request(request_method method,
                        std::regex route,
                        endpoint_handler_cb callback,
                        parameter::validators validations = {});

    void handle_request(request_method method,
                        std::string route,
                        endpoint_handler_cb callback,
                        parameter::validators validations = {});

    void serve_files(std::string mount_point, std::string path_to_files);

    void add_header(std::string &&key, std::string &&value);

    // for use by the Server object
    std::experimental::optional<luna::response> process_request(request &request);

    // options setters
    template<typename T>
    void set_options_(T &&t)
    {
        set_option_(LUNA_FWD(t));
    }

    template<typename T, typename... Ts>
    void set_options_(T &&t, Ts &&... ts)
    {
        set_options_(LUNA_FWD(t));
        set_options_(LUNA_FWD(ts)...);
    }

    void set_option_(mime_type mime_type);

private:
    class router_impl;
    std::shared_ptr<router_impl> impl_;

    void initialize_(std::string route_base);
};


} //namespace luna