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
    router_impl(std::string route_base) : route_base_{std::forward<std::string>(route_base)}
    {}

    std::string route_base_;
    std::mutex lock_;
    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    std::map<request_method, request_handlers> request_handlers_;
    luna::headers headers_;
};

} //namespace luna