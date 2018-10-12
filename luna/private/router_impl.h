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
    router_impl(std::string route_base) :
            route_base_{std::move(route_base)},
            mime_type_{"text/html; charset=UTF-8"}
    {
        //remove trailing slashes
        if (route_base_.back() == '/')
        {
            route_base_.pop_back();
        }
    }

    std::string route_base_;
    std::mutex lock_;
    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    std::map<request_method, request_handlers> request_handlers_;
    luna::headers headers_;
    std::string mime_type_;
};

} //namespace luna
