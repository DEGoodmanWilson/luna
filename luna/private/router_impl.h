//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
//                   )
//
// Luna
// A web application and API framework in modern C++
//
// Copyright © 2016–2018 D.E. Goodman-Wilson
//

#pragma once

#include <luna/router.h>
#include <map>
#include <vector>
#include <tuple>
#include <mutex>

namespace luna
{


class router::router_impl
{
public:
    router_impl(std::string route_base);


    void set_mime_type(std::string mime_type);

    using endpoint_handler_cb = std::function<response (const request &req)>;

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

    OPT_NS::optional<luna::response> process_request(request &request);

private:

    std::string route_base_;
    std::mutex lock_;
    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    std::map<request_method, request_handlers> request_handlers_;
    luna::headers headers_;
    std::string mime_type_;
};

} //namespace luna
