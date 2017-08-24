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

const server::error_handler_cb default_error_handler_callback_ = [](const request &request,
                                                                    response &response)
{
    if (response.content.empty())
    {
        response.content_type = "text/html; charset=UTF-8";
        //we'd best render it ourselves.
        // TODO move these into error_handlers maybe?
        switch (response.status_code)
        {
            case 404:
                response.content = "<h1>Not found</h1>";
                break;
            default:
                response.content = "<h1>So sorry, generic server error</h1>";
        }
    }
};

//void add_uniform_headers_(std::shared_ptr<cacheable_response> response)
//{
//
//}

//////////////////////////////////////////////////////////////////////////////


response_generator::response_generator() :
        error_handler_callback_{default_error_handler_callback_},
        default_mime_type_{"text/html; charset=UTF-8"},
        server_identifier_{std::string{LUNA_NAME} + "/" + LUNA_VERSION}
{
    // load up the pre-generated responses
//    auto response_404 = std::make_shared<cacheable_response>(
//            MHD_create_response_from_buffer
//    )
}

//std::shared_ptr<cacheable_response> response_generator::fetch_error_response(const status_code code)
//{
//
//}

std::shared_ptr<cacheable_response>
response_generator::generate_response(const request &request, response &response)
{
    // First we need to decide if this response object represents a file on disk, or a buffer in memory.
    if (!response.file.empty())
    {
        return from_file_(request, response);
    }

    // Add default status code, if missing
    // TODO we can DRY this up too I bet
    if (0 == response.status_code)
    {
        response.status_code = default_success_code_(request.method);
    }


    // TODO more drying things up
    // render the error response
    if (is_error_(response.status_code))
    {
        // render the error
        error_handler_callback_(request, response);

        // custom error handlers
        if (error_handlers_.count(response.status_code))
        {
            error_handlers_.at(response.status_code)(request, response); //re-render response
        }

        // and call the after_error middlewares
        for (const auto &mw : middleware_after_error_.funcs)
        {
            // TODO update the interface to accept requests.
            mw(response);
        }
    }

    // add mime type if needed
    // TODO can we DRY this code up?
    if (response.content_type.empty()) //no content type assigned, use the default
    {
        response.content_type = default_mime_type_;
    }




    // Now, create the MHD_Response object
    // TODO it would be nice if we could cache this!
    auto response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                             (void *) response.content.c_str(),
                                                                                             MHD_RESPMEM_MUST_COPY),
                                                             response.status_code);

    // Add headers to response object
    for (const auto &header : response.headers)
    {
        MHD_add_response_header(response_mhd->mhd_response, header.first.c_str(), header.second.c_str());
    }
    for (const auto &header : global_headers_)
    {
        MHD_add_response_header(response_mhd->mhd_response, header.first.c_str(), header.second.c_str());
    }
    MHD_add_response_header(response_mhd->mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE, response.content_type.c_str());
    MHD_add_response_header(response_mhd->mhd_response, MHD_HTTP_HEADER_SERVER, server_identifier_.c_str());

    // TODO can we cache this response?
    return response_mhd;
};


std::shared_ptr<cacheable_response>
response_generator::from_file_(const request &request, response &response)
{
    std::string content{"NOPE"};
    auto my_response = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(content.length(),
                                                                                            (void *) content.c_str(),
                                                                                            MHD_RESPMEM_MUST_COPY),
                                                            500);

    return my_response;
};



///// Option setters
void response_generator::set_option(server::error_handler_cb handler)
{
    error_handler_callback_ = handler;
}

void response_generator::set_option(const server::server_identifier &value)
{
    server_identifier_ = value;
}

void response_generator::set_option(const server::append_to_server_identifier &value)
{
    server_identifier_ += " " + value;
}

void response_generator::set_option(const server::mime_type &mime_type)
{
    default_mime_type_ = mime_type;
}

void response_generator::set_option(middleware::after_error value)
{
    middleware_after_error_ = value;
}


void response_generator::add_global_header(std::string &&header, std::string &&value)
{
    global_headers_.emplace(std::move(header), std::move(value));
}

void response_generator::add_global_header(const std::string &header, std::string &&value)
{
    global_headers_.emplace(header, std::move(value));
}

void response_generator::add_global_header(std::string &&header, const std::string &value)
{
    global_headers_.emplace(std::move(header), value);
}

void response_generator::add_global_header(const std::string &header, const std::string &value)
{
    global_headers_.emplace(header, value);
}

server::error_handler_handle response_generator::handle_error(status_code code, server::error_handler_cb callback)
{
    error_handlers_[code] = callback;
    return code;
}

void response_generator::remove_error_handler(server::error_handler_handle item)
{
    //TODO this is expensive. Find a better way to store this stuff.
    //TODO validate we are receiving a valid iterator!!
    error_handlers_.erase(item);
}


//
/*
bool response_generator::render_error_(request & request, response & response, MHD_Connection * connection)
{
    // unsupported HTTP method
error_handler_callback_(request, response); //hook for modifying response

// get custom error page if exists
if (error_handlers_.count(response.status_code))
{
error_handlers_.at(response.status_code)(request, response); //re-render response
}

return render_response_(request, response, connection);
}
*/


} //namespace luna