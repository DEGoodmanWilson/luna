//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#include "response_generator.h"

namespace luna
{

bool is_error_(status_code code)
{
    if (code < 300) return false;

    return true;
}

status_code default_success_code_(request_method method)
{
    if (method == request_method::POST)
    {
        return 201;
    }

    return 200;
}

//void add_uniform_headers_(std::shared_ptr<cacheable_response> response)
//{
//
//}

response_generator::response_generator() : default_mime_type_{"text/html; charset=UTF-8"}
{
    // load up the pre-generated responses
//    auto response_404 = std::make_shared<cacheable_response>(
//            MHD_create_response_from_buffer
//    )
}

//std::shared_ptr<cacheable_response> response_generator::fetch_error_response(const luna::status_code code)
//{
//
//}

std::shared_ptr<cacheable_response>
response_generator::generate_response(const luna::request &request, const luna::response &response)
{
    // First we need to decide if this response object represents a file on disk, or a buffer in memory.
    if (!response.file.empty())
    {
        return from_file_(request, response);
    }

    // Add default status code, if missing
    auto status_code = response.status_code;
    if (0 == status_code)
    {
        status_code = default_success_code_(request.method);
    }


    // That was easy!
    // TODO is there some way we can pull this out of a cache somewhere?
    auto my_response = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                            (void *) response.content.c_str(),
                                                                                            MHD_RESPMEM_MUST_COPY),
                                                            status_code);


    // add mime type if needed
    // TODO can we DRY this code up?
    std::string content_type{response.content_type};
    if (content_type.empty()) //no content type assigned, use the default
    {
        content_type = default_mime_type_;
    }

    // Add headers to response object
    // TODO we need to DRY this code up
    for (const auto &header : response.headers)
    {
        MHD_add_response_header(my_response->mhd_response, header.first.c_str(), header.second.c_str());
    }
    MHD_add_response_header(my_response->mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE, content_type.c_str());
//    MHD_add_response_header(my_response->mhd_response, MHD_HTTP_HEADER_SERVER, server_identifier_.c_str());

    // TODO can we cache this response?
    return my_response;
};


std::shared_ptr<cacheable_response> response_generator::from_file_(const luna::request &request, const luna::response &response)
{
    std::string content{"NOPE"};
    auto my_response = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(content.length(),
                                                                                            (void *) content.c_str(),
                                                                                            MHD_RESPMEM_MUST_COPY),
                                                            500);

    return my_response;
};


} //namespace luna