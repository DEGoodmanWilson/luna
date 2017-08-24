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
    MAKE_BOOL_LIKE(start_on_construction);

    MAKE_BOOL_LIKE(debug_output);

    MAKE_BOOL_LIKE(use_ssl);

    MAKE_BOOL_LIKE(use_thread_per_connection);

    MAKE_BOOL_LIKE(use_epoll_if_available);

    MAKE_INT_LIKE(int, port);

    MAKE_STRING_LIKE(mime_type);

    using accept_policy_cb = std::function<bool(const struct sockaddr *add, socklen_t len)>;


    using endpoint_handler_cb = std::function<response(const request &req)>;

    using error_handler_cb = std::function<void(const request &request,
                                                response &response //a hook for modifying in place to insert default content
                                                )>;
    // MHD config options

    using unescaper_cb = std::function<std::string(const std::string &text)>;

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

    MAKE_STRING_LIKE(server_identifier);
    MAKE_STRING_LIKE(append_to_server_identifier);

    MAKE_BOOL_LIKE(enable_internal_file_caching);


    server()
    {
        initialize_();
        start();
    }

    template<typename ...Os>
    server(Os &&...os) : start_on_construct_{true}
    {
        initialize_();
        set_options_(LUNA_FWD(os)...);
        if(start_on_construct_)
        {
            start();
        }
    }

    ~server();

    void start();
    void stop();
    void await();

    server::port get_port();

    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    using request_handler_handle = std::pair<request_method, request_handlers::const_iterator>;

    using error_handler_handle = status_code;

    template<typename T>
    request_handler_handle handle_request(request_method method, T&& path, endpoint_handler_cb callback)
    {
        return handle_request(method, std::regex{std::forward<T>(path)}, callback, {});
    }
    template<typename T>
    request_handler_handle handle_request(request_method method, T&& path, endpoint_handler_cb callback, parameter::validators &&validations)
    {
        return handle_request(method, std::regex{std::forward<T>(path)}, callback, std::forward<parameter::validators>(validations));
    }
    template<typename T>
    request_handler_handle handle_request(request_method method, T&& path, endpoint_handler_cb callback, const parameter::validators &validations)
    {
        return handle_request(method, std::regex{std::forward<T>(path)}, callback, validations);
    }
    request_handler_handle handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback, parameter::validators &&validations);
    request_handler_handle handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback, parameter::validators &&validations);
    request_handler_handle handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback, const parameter::validators &validations);
    request_handler_handle handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback, const parameter::validators &validations);

    request_handler_handle serve_files(const std::string &mount_point, const std::string &path_to_files);
    request_handler_handle serve_files(std::string &&mount_point, std::string &&path_to_files);

    void remove_request_handler(request_handler_handle item);

    error_handler_handle handle_404(error_handler_cb callback);
    error_handler_handle handle_error(status_code code, error_handler_cb callback);

    void remove_error_handler(error_handler_handle item);

    // TODO is this really necessary? It seems overmuch.
    void add_global_header(std::string &&header, std::string &&value);

    void add_global_header(const std::string &header, std::string &&value);

    void add_global_header(std::string &&header, const std::string &value);

    void add_global_header(const std::string &header, const std::string &value);


    explicit operator bool();

private:
    class server_impl;

    struct server_impl_deleter
    {
        void operator()(server_impl *) const;
    };

    std::unique_ptr<server_impl, server_impl_deleter> impl_;

    bool start_on_construct_;

    void initialize_();

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

    void set_option_(start_on_construction value);

    void set_option_(debug_output value);

    void set_option_(use_thread_per_connection value);

    void set_option_(use_epoll_if_available value);

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

//    void set_option_(tcp_fastopen_queue_size value);

    void set_option_(const https_mem_dhparams &value);

//    void set_option_(listening_address_reuse value);

    void set_option_(const https_key_password &value);

//    void set_option_(notify_connection value); //TODO later

    void set_option_(const server_identifier &value);
    void set_option_(const append_to_server_identifier &value);

    // middleware
    void set_option_(middleware::before_request_handler value);
    void set_option_(middleware::after_request_handler value);
    void set_option_(middleware::after_error value);

    // user-provided static asset caching
    void set_option_(std::pair<cache::read, cache::write> value);
    // internally-provided static asset caching
    void set_option_(enable_internal_file_caching value);
};

//for testing purposes only.
std::string string_format(const std::string fmt_str, ...);

} //namespace luna
