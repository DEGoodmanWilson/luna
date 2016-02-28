//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <sys/socket.h>
#include <functional>
#include <microhttpd.h>
#include <memory>

namespace luna
{


class server
{
public:

    // configuration parameters
    MAKE_UINT16_T_LIKE(port);

    MAKE_STRING_LIKE(mime_type);

    using accept_policy_cb = std::function<bool(const struct sockaddr *, socklen_t)>;


    using endpoint_handler_cb = std::function<response(const endpoint_matches &matches,
                                                       const query_params &params)>;

    using error_handler_cb = std::function<void(response &response, //a hook for modifying in place to insert default content
                                                request_method method,
                                                const std::string &path)>;
    // MHD config options

    using logger_cb = std::function<void(const std::string& message)>;

    using unescaper_cb = std::function<std::string(std::string)>;

    //TODO just not going to try to support these two for now
    //TODO MHD_OPTION_HTTPS_CERT_CALLBACK cbshim_
    //    using notify_connection_cb = std::function<void(struct MHD_Connection *connection, void **socket_context, enum MHD_ConnectionNotificationCode toe)>;


    MAKE_INT_LIKE(size_t, connection_memory_limit);

    MAKE_INT_LIKE(unsigned int, connection_limit);

    MAKE_INT_LIKE(unsigned int, connection_timeout);

    MAKE_INT_LIKE(unsigned int, per_ip_connection_limit);

    using sockaddr_ptr = ::sockaddr*;
    // struct sockaddr * is a configuration option here! Just letting you know.

    MAKE_STRING_LIKE(https_mem_key);

    MAKE_STRING_LIKE(https_mem_cert);

//    MAKE_INT_LIKE(gnutls_credentials_type_t, https_cred_type); //TODO probably don't need to define this one.

    MAKE_STRING_LIKE(https_priorities);

    MAKE_INT_LIKE(int, listen_socket);

    MAKE_INT_LIKE(unsigned int, thread_pool_size);

//    MAKE_INT_LIKE(unsigned int, digest_auth_random); //TODO unsure how best to support this one

    MAKE_INT_LIKE(unsigned int, nonce_nc_size);

    MAKE_INT_LIKE(size_t, thread_stack_size);

    MAKE_STRING_LIKE(https_mem_trust);

    MAKE_INT_LIKE(size_t, connection_memory_increment);

    MAKE_INT_LIKE(unsigned int, tcp_fastopen_queue_size);

    MAKE_STRING_LIKE(https_mem_dhparams);

    MAKE_INT_LIKE(unsigned int, listening_address_reuse);

    MAKE_STRING_LIKE(https_key_password);


    template
    server()
    {
        initialize_();
        start_();
    }

    template<typename ...Os>
    server(Os &&...os)
    {
        initialize_();
        set_options_(LUNA_FWD(os)...);
        start_();
    }

    ~server();

    server::port get_port();


    template<typename T>
    void handle_request(request_method method, T path, endpoint_handler_cb callback)
    {
        handle_request(method, std::regex{std::forward<T>(path)}, callback);
    }

    template
    void handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback);

    template
    void handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback);

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

    void set_option_(accept_policy_cb handler);

    //MHD options
    void set_option_(connection_memory_limit value);

    void set_option_(connection_limit value);

    void set_option_(connection_timeout value);

//    void set_option_(notify_completed value);

    void set_option_(per_ip_connection_limit value);

    void set_option_(const sockaddr_ptr value);

    void set_option_(logger_cb value);

    void set_option_(https_mem_key value);

    void set_option_(https_mem_cert value);

//    void set_option_(https_cred_type value); //TODO later

    void set_option_(https_priorities value);

    void set_option_(listen_socket value);

    void set_option_(thread_pool_size value);

    void set_option_(unescaper_cb value);

//    void set_option_(digest_auth_random value); //TODO later

    void set_option_(nonce_nc_size value);

    void set_option_(thread_stack_size value);

    void set_option_(const https_mem_trust &value);

    void set_option_(connection_memory_increment value);

//    void set_option_(https_cert_callback value); //TODO later

    void set_option_(tcp_fastopen_queue_size value);

    void set_option_(const https_mem_dhparams &value);

    void set_option_(listening_address_reuse value);

    void set_option_(const https_key_password &svalue);

//    void set_option_(notify_connection value); //TODO later

};

} //namespace luna
