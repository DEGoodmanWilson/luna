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

#include <luna/types.h>
#include <microhttpd.h>
#include <unordered_map>
#include <chrono>

namespace luna
{

struct cacheable_response
{
    struct MHD_Response *mhd_response;
    luna::status_code status_code;

    bool cached;
    std::chrono::system_clock::time_point time_cached;

    cacheable_response(struct MHD_Response *mhd_response, luna::status_code status_code);

    ~cacheable_response();
};

} //namespace luna