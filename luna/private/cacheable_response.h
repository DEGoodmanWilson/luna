//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <microhttpd.h>
#include <unordered_map>

namespace luna
{

struct cacheable_response
{
    struct MHD_Response *mhd_response;
    luna::status_code status_code;

    cacheable_response(struct MHD_Response *mhd_response, luna::status_code status_code);

    ~cacheable_response();
};

} //namespace luna