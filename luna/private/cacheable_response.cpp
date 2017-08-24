//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#include "cacheable_response.h"

namespace luna
{

cacheable_response::cacheable_response(struct MHD_Response *mhd_response, luna::status_code status_code)
        : mhd_response{mhd_response}, status_code{status_code}, cached{false}
{
}

cacheable_response::~cacheable_response()
{
    if (mhd_response != nullptr)
    {
        MHD_destroy_response(mhd_response);
    }
}

} //namespace luna