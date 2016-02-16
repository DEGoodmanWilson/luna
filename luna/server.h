//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <sys/socket.h>
#include <string>
#include <regex>
#include <memory>
#include <map>
#include <functional>
#include <microhttpd.h>

namespace luna
{


class server
{
public:

    // configuration parameters
    MAKE_UINT16_T_LIKE(port);

    MAKE_STRING_LIKE(mime_type);

    using access_policy_cb = std::function<bool(const struct sockaddr *, socklen_t)>;


    using endpoint_handler_cb = std::function<status_code(std::vector<std::string> matches,
                                                          query_params params,
                                                          response &body_out)>;

    using error_handler_cb = std::function<response(uint16_t error_code,
                                                    request_method method,
                                                    const std::string &path)>;
    // MHD config options
    MAKE_INT_LIKE(size_t, connection_memory_limit);

    MAKE_INT_LIKE(unsigned int, connection_limit);

    MAKE_INT_LIKE(unsigned int, connection_timeout);

    using notify_completed_handler_cb = std::function<>; //TODO

    MAKE_INT_LIKE(unsigned int, per_ip_connection_limit);

//    MAKE_INT_LIKE(unsigned int, sock_addr); //TODO struct sockaddr *

//    MAKE_INT_LIKE(unsigned int, uri_log_callback); //void * my_logger(void *cls, const char *uri, struct MHD_Connection *con) TODO do this one like the callbacks above

    MAKE_STRING_LIKE(https_mem_key);

    MAKE_STRING_LIKE(https_mem_cert);

//    MAKE_INT_LIKE(gnutls_credentials_type_t, https_cred_type); //TODO probably don't need to define this one.

    MAKE_STRING_LIKE(https_priorities);

    MAKE_INT_LIKE(int, listen_socket);

//    MAKE_INT_LIKE(unsigned int, external_logger); //TODO another two-parameter beast. We need to redefine this one pretty good

    MAKE_INT_LIKE(unsigned int, thread_pool_size);

//    MAKE_INT_LIKE(unsigned int, unescape_callback); //TODO callback

//    MAKE_INT_LIKE(unsigned int, digest_auth_random); //TODO two args, size_t and a pointer that gets dealloc'd later. Make it a unique_ptr

    MAKE_INT_LIKE(unsigned int, nonce_nc_size);

    MAKE_INT_LIKE(size_t, thread_stack_size);

    MAKE_STRING_LIKE(https_mem_trust);

    MAKE_INT_LIKE(size_t, connection_memory_increment);

//    MAKE_INT_LIKE(unsigned int, https_cert_callback); //ugh a callback of gnutls_certificate_retrieve_function2 *

    MAKE_INT_LIKE(unsigned int, tcp_fastopen_queue_size);

    MAKE_STRING_LIKE(https_mem_dhparams);

    MAKE_INT_LIKE(unsigned int, listening_address_reuse);

    MAKE_STRING_LIKE(https_key_password);

//    MAKE_INT_LIKE(unsigned int, notify_connection); //Anohter callback with a void* closure


    template<typename ...Os>
    server(Os &&...os)
    {
        initialize_();
        set_options_(LUNA_FWD(os)...);
        start_();
    }

    ~server();


    template<typename T>
    void handle_response(request_method method, T path, endpoint_handler_cb callback)
    {
        handle_response(method, std::regex{std::forward<T>(path)}, callback);
    }

    template
    void handle_response(request_method method, const std::regex &path, endpoint_handler_cb callback);

    template
    void handle_response(request_method method, std::regex &&path, endpoint_handler_cb callback);

    explicit operator bool();

private:
    class server_impl;

    struct server_impl_deleter
    {
        void operator()(server_impl *) const;
    };

    std::unique_ptr<server_impl, server_impl_deleter> impl_;


    void initialize_();

    void start_();

    template<typename T>
    void set_options_(T &&t)
    {
        set_option_(LUNA_FWD(t));
    }

    template<typename T, typename... Ts>
    void set_options_(T &&t, Ts &&... ts)
    {
        set_options_(LUNA_FWD(t));
        set_options_(LUNA_FWD(ts)...);
    }


    void set_option_(mime_type mime_type);

    void set_option_(error_handler_cb handler);

    void set_option_(server::port port);

    void set_option_(access_policy_cb handler);

    //MHD options TODO not all of these are ints
    void set_option_(connection_memory_limit value);

    void set_option_(connection_limit value);

    void set_option_(connection_timeout value);

//    void set_option_(notify_completed value);

    void set_option_(per_ip_connection_limit value);

    void set_option_(const sockaddr *value);

//    void set_option_(uri_log_callback value);

    void set_option_(https_mem_key value);

    void set_option_(https_mem_cert value);

//    void set_option_(https_cred_type value);

    void set_option_(https_priorities value);

    void set_option_(listen_socket value);

//    void set_option_(external_logger value);

    void set_option_(thread_pool_size value);

//    void set_option_(unescape_callback value);

//    void set_option_(digest_auth_random value);

    void set_option_(nonce_nc_size value);

    void set_option_(thread_stack_size value);

    void set_option_(const https_mem_trust &value);

    void set_option_(connection_memory_increment value);

//    void set_option_(https_cert_callback value);

    void set_option_(tcp_fastopen_queue_size value);

    void set_option_(const https_mem_dhparams &value);

    void set_option_(listening_address_reuse value);

    void set_option_(const https_key_password &svalue);

//    void set_option_(notify_connection value);

};

} //namespace luna
