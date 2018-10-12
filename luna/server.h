//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
//                   )
//
// Luna
// A web application and API framework in modern C++
//
// Copyright © 2016–2018 D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <luna/router.h>
#include <sys/socket.h>
#include <functional>
#include <microhttpd.h>
#include <memory>
#include <chrono>

namespace luna
{

class server
{
public:

    // configuration parameters
    MAKE_LIKE(bool, debug_output);

    MAKE_LIKE(bool, use_ssl);

    MAKE_LIKE(bool, use_thread_per_connection);

    MAKE_LIKE(bool, use_epoll_if_available);

    using accept_policy_cb = std::function<bool(const struct sockaddr *add, socklen_t len)>;


    // MHD config options

    using unescaper_cb = std::function<std::string(const std::string &text)>;

    MAKE_LIKE(size_t, connection_memory_limit);

    MAKE_LIKE(unsigned int, connection_limit);

    MAKE_LIKE(unsigned int, connection_timeout);

    MAKE_LIKE(unsigned int, per_ip_connection_limit);

    using sockaddr_ptr = ::sockaddr *;
    // struct sockaddr * is a configuration option here! Just letting you know.

    MAKE_LIKE(std::string, https_mem_key);

    MAKE_LIKE(std::string, https_mem_cert);

//    MAKE_LIKE(gnutls_credentials_type_t, https_cred_type); //TODO probably don't need to define this one.

    MAKE_LIKE(std::string, https_priorities);

    MAKE_LIKE(int, listen_socket);

    MAKE_LIKE(unsigned int, thread_pool_size);

//    MAKE_LIKE(unsigned int, digest_auth_random); //TODO unsure how best to support this one

    MAKE_LIKE(unsigned int, nonce_nc_size);

    MAKE_LIKE(size_t, thread_stack_size);

    MAKE_LIKE(std::string, https_mem_trust);

    MAKE_LIKE(size_t, connection_memory_increment);

    MAKE_LIKE(unsigned int, tcp_fastopen_queue_size);

    MAKE_LIKE(std::string, https_mem_dhparams);

    MAKE_LIKE(unsigned int, listening_address_reuse);

    MAKE_LIKE(std::string, https_key_password);

    MAKE_LIKE(std::string, server_identifier);

    using server_identifier_and_version = std::pair<std::string, std::string>;

    MAKE_LIKE(std::string, append_to_server_identifier);

    MAKE_LIKE(bool, enable_internal_file_cache);

    using internal_file_cache_keep_alive = std::chrono::milliseconds;

    server()
    {
        initialize_();
    }

    template<typename ...Os>
    server(Os &&...os)
    {
        initialize_();
        set_options_(LUNA_FWD(os)...);
    }

    ~server();

    server(const server&) = delete;
    server& operator=(server&&); // move constructor defined in the implementation file
    server& operator=(const server&) = delete;

    bool start(uint16_t port = 8080);

    bool start_async(uint16_t port = 8080);

    bool is_running();

    void stop();

    void await();

    uint16_t get_port();

    std::shared_ptr<router> create_router(std::string route_base = "/");

    explicit operator bool();

private:
    class server_impl;

    // we have to define the deleter here, because we have to inline the constructors above, because templates.
    struct server_impl_deleter
    {
        void operator()(server_impl *) const;
    };

//    std::experimental::propagate_const<std::unique_ptr<server_impl>> impl_;
    std::unique_ptr<server_impl, server_impl_deleter> impl_;

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

    void set_option_(debug_output value);

    void set_option_(use_thread_per_connection value);

    void set_option_(use_epoll_if_available value);

    void set_option_(accept_policy_cb handler);

    //MHD options
    void set_option_(connection_memory_limit value);

    void set_option_(connection_limit value);

    void set_option_(connection_timeout value);

//    void set_option_(notify_completed value);

    void set_option_(per_ip_connection_limit value);

    void set_option_(const sockaddr_ptr value);

    void set_option_(const https_mem_key &value);

    void set_option_(const https_mem_cert &value);

//    void set_option_(const https_cred_type &value); //TODO later

    void set_option_(const https_priorities &value);

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
    void set_option_(const server_identifier_and_version &value);

    void set_option_(const append_to_server_identifier &value);

    // internally-provided static asset caching
    void set_option_(enable_internal_file_cache value);

    void set_option_(internal_file_cache_keep_alive value);
};

} //namespace luna
