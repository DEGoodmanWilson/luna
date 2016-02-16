//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#pragma once

#include "server.h"
#include <microhttpd.h>
#include <cstring>
#include <iostream>


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

    server_impl();

    ~server_impl();

    void start();
    bool is_running();
    void stop();

    void handle_response(request_method method, const std::regex &path, endpoint_handler_cb callback);

    void set_option(const mime_type &mime_type);

    void set_option(error_handler_cb handler);

    void set_option(server::port port);

    void set_option(access_policy_cb handler);

    // MHD specific options

    void set_option(connection_memory_limit value);

    void set_option(connection_limit value);

    void set_option(connection_timeout value);

//    void set_option(notify_completed value);

    void set_option(per_ip_connection_limit value);

    void set_option(const sockaddr *value);

//    void set_option(uri_log_callback value);

    void set_option(const https_mem_key &value);

    void set_option(const https_mem_cert &value);

//    void set_option(https_cred_type value);

    void set_option(const https_priorities &value);

    void set_option(listen_socket value);

//    void set_option(external_logger value);

    void set_option(thread_pool_size value);

//    void set_option(unescape_callback value);

//    void set_option(digest_auth_random value);

    void set_option(nonce_nc_size value);

    void set_option(thread_stack_size value);

    void set_option(const https_mem_trust &value);

    void set_option(connection_memory_increment value);

//    void set_option(https_cert_callback value);

    void set_option(tcp_fastopen_queue_size value);

    void set_option(const https_mem_dhparams &value);

    void set_option(listening_address_reuse value);

    void set_option(const https_key_password &value);

//    void set_option(notify_connection value);

private:
    std::map<request_method, std::vector<std::pair<std::regex, endpoint_handler_cb>>> response_handlers_;

    uint16_t port_;
    error_handler_cb error_handler_;
    access_policy_cb access_policy_handler_;


    std::vector<std::string> options_string_cache_;
    std::vector<MHD_OptionItem> options_;

    struct MHD_Daemon *daemon_;


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
