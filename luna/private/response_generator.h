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

    std::shared_ptr<cacheable_response> generate_response(const luna::request &request, luna::response &response);

    // option setters
    void set_option(server::error_handler_cb handler);
    void set_option(const server::server_identifier &value);
    void set_option(const server::append_to_server_identifier &value);
    void set_option(const server::mime_type &mime_type);
    void set_option(middleware::after_error value);

    // more options
    void add_global_header(std::string &&header, std::string &&value);
    void add_global_header(const std::string &header, std::string &&value);
    void add_global_header(std::string &&header, const std::string &value);
    void add_global_header(const std::string &header, const std::string &value);

    // error renderers
    server::error_handler_handle handle_error(status_code code, server::error_handler_cb callback);
    void remove_error_handler(server::error_handler_handle item);

//    server::error_handler_handle handle_404(server::error_handler_cb callback);
//    server::error_handler_handle handle_error(status_code code, server::error_handler_cb callback);
//    void remove_error_handler(error_handler_handle item);


private:
    std::shared_ptr<cacheable_response> from_file_(const luna::request &request, luna::response &response);

//    std::string server_identifier_;
//    std::unordered_map<std::string, std::shared_ptr<cacheable_response> > fd_cache_;

    std::unordered_map<status_code, cacheable_response> error_respone_cache_;

    std::string default_mime_type_;
    std::string server_identifier_;
    luna::headers global_headers_;

    server::error_handler_cb error_handler_callback_;
    std::map<status_code, server::error_handler_cb> error_handlers_;
    middleware::after_error middleware_after_error_;
};



} //namespace luna