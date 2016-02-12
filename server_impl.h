//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#pragma once

#include "server.h"
#include <microhttpd.h>
#include <cstring>

namespace luna
{

const auto GET = "GET";
const auto POST = "POST";
const auto PUT = "PUT";
const auto PATCH = "PATCH";
const auto DELETE = "DELETE";

static const server::error_handler_cb default_error_handler_ = [](uint16_t error_code,
                                                                  request_method method,
                                                                  const std::string &path) -> response
    {
        std::string content_type{"text/html"};
        //we'd best render it ourselves.
        switch (error_code)
        {
            case 404:
                return {content_type, "<h1>Not found</h1>"};
            default:
                return {content_type, "<h1>So sorry, generic server error</h1>"};
        }

    };

static const server::access_policy_cb default_access_policy_handler_ = [](const struct sockaddr *addr,
                                                                            socklen_t len) -> bool
    {
        return true;
    };

class server::server_impl
{
public:

    server_impl() : daemon_{nullptr}, error_handler_{default_error_handler_}, access_policy_handler_{default_access_policy_handler_}
    { }

    ~server_impl();

    void handle_response(request_method method, const std::regex &path, endpoint_handler_cb callback);

    bool start();

    void set_option(mime_type mime_type);

    void set_option(error_handler_cb handler);

    void set_option(server::port port);

    void set_option(access_policy_cb handler);

private:
    struct MHD_Daemon *daemon_;
    std::map<request_method, std::vector<std::pair<std::regex, endpoint_handler_cb>>> response_handlers_;

    uint16_t port_;
    error_handler_cb error_handler_;
    access_policy_cb access_policy_handler_;

    static int parse_kv_(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);

    int access_handler_callback_(struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method,
                                 const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size,
                                 void **con_cls);


    static int access_policy_callback_shim_(void *cls,
                                            const struct sockaddr *addr,
                                            socklen_t addrlen);

    static int access_handler_callback_shim_(void *cls,
                                             struct MHD_Connection *connection,
                                             const char *url,
                                             const char *method,
                                             const char *version,
                                             const char *upload_data,
                                             size_t *upload_data_size,
                                             void **con_cls);

    int render_response_(status_code status_code,
                         response resp,
                         MHD_Connection *connection,
                         const char *url,
                         request_method method) const;

    int render_error_(uint16_t error_cpde, MHD_Connection *connection, const char *url, request_method method) const;

};

} //namespace luna
