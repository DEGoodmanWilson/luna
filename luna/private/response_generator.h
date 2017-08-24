//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#pragma once

#include <unordered_map>
#include <string>
#include <luna/luna.h>
#include <unordered_map>
#include "luna/private/cacheable_response.h"

namespace luna
{

class response_generator
{
public:
    response_generator();

    std::shared_ptr<cacheable_response> generate_response(const luna::request &request, const luna::response &response);
//    std::shared_ptr<cacheable_response> fetch_error_response(const luna::status_code code);


private:
    std::shared_ptr<cacheable_response> from_file_(const luna::request &request, const luna::response &response);

//    std::string server_identifier_;
//    std::unordered_map<std::string, std::shared_ptr<cacheable_response> > fd_cache_;

    std::string default_mime_type_;

};



} //namespace luna