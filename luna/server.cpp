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

#include "server.h"
#include "server_impl.h"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "luna/private/server_impl.h"
#include "luna/config.h"
#include "luna/optional.hpp"

#define LOG_FATAL(mesg) \
{ \
    error_log(log_level::FATAL, mesg); \
}

#define LOG_ERROR(mesg) \
{ \
    error_log(log_level::ERROR, mesg); \
}

#define LOG_INFO(mesg) \
{ \
    error_log(log_level::INFO, mesg); \
}

#define LOG_DEBUG(mesg) \
{ \
    error_log(log_level::DEBUG, mesg); \
}


static const auto GET = "GET";
static const auto POST = "POST";
static const auto PUT = "PUT";
static const auto PATCH = "PATCH";
static const auto DELETE = "DELETE";
static const auto OPTIONS = "OPTIONS";

namespace luna
{

void server::server_impl_deleter::operator()(server::server_impl *ptr) const
{ delete ptr; }


server::operator bool()
{
    return (impl_->daemon_ != nullptr);
}

void server::initialize_()
{
    impl_.reset(new server_impl());
}

server::~server()
{
    stop();
}

// TODO
// * multiple headers with same name
// * catch exceptions in user logger and middleware functions, throw 500 when they happen.

///////////////////////////




bool server::start(uint16_t port)
{
    auto result = start_async(port);

    if (!result) return result;

    // TODO would be better to find a way to run MHD in _this_ thread...
    await();

    return true;
}

bool server::start_async(uint16_t port)
{
    impl_->port_ = port;

    MHD_OptionItem options[impl_->options_.size() + 1];
    uint16_t idx = 0;
    for (const auto &opt : impl_->options_)
    {
        options[idx++] = opt; //copy it in, whee.
    }
    options[idx] = {MHD_OPTION_END, 0, nullptr};

    unsigned int flags = MHD_NO_FLAG;

    if (impl_->debug_output_)
    {
        LOG_DEBUG("Enabling debug output");
        flags |= MHD_USE_DEBUG;
    }

    if (impl_->ssl_mem_cert_set_ && impl_->ssl_mem_key_set_)
    {
        LOG_DEBUG("Enabling SSL");
        flags |= MHD_USE_SSL;
    }
    else if (impl_->ssl_mem_cert_set_ || impl_->ssl_mem_key_set_)
    {
        LOG_FATAL("Please provide both server::https_mem_key AND server::https_mem_cert");
        return false;
    }

    if (impl_->use_thread_per_connection_)
    {
        LOG_DEBUG("Will use one thread per connection")
        flags |= MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL;
    }
    else if (impl_->use_epoll_if_available_)
    {
#if defined(__linux__)
        LOG_DEBUG("Will use epoll");
        flags |= MHD_USE_EPOLL_INTERNALLY;
#else
        LOG_DEBUG("Will use poll");
        flags |= MHD_USE_POLL_INTERNALLY;
#endif
    }
    else
    {
        LOG_DEBUG("No threading options set, will use select");
        flags |= MHD_USE_SELECT_INTERNALLY;
    }

    impl_->daemon_ = MHD_start_daemon(flags,
                                      impl_->port_,
                                      impl_->access_policy_callback_shim_, impl_.get(),
                                      impl_->access_handler_callback_shim_, impl_.get(),
                                      MHD_OPTION_NOTIFY_COMPLETED, impl_->request_completed_callback_shim_, impl_.get(),
                                      MHD_OPTION_EXTERNAL_LOGGER, impl_->logger_callback_shim_, nullptr,
                                      MHD_OPTION_URI_LOG_CALLBACK, impl_->uri_logger_callback_shim_, nullptr,
                                      MHD_OPTION_ARRAY, options,
                                      MHD_OPTION_END);

    if (!impl_->daemon_)
    {
        LOG_FATAL(impl_->server_name_ + " server failed to start (are you already running something on port " + std::to_string(impl_->port_) +
                  "?)"); //TODO set some real error flags perhaps?
        return false;
    }
    impl_->running_cv_.notify_all(); //impl_->daemon_ has changed value

    LOG_INFO(impl_->server_name_ + " server created on port " + std::to_string(impl_->port_));

    return true;
}


bool server::is_running()
{
    return (impl_->daemon_ != nullptr);
}

void server::stop()
{
    if (impl_->daemon_)
    {
        MHD_stop_daemon(impl_->daemon_);
        LOG_INFO(impl_->server_name_ + " server stopped");
        impl_->daemon_ = nullptr;
        impl_->running_cv_.notify_all(); //impl_->daemon_ has changed value
    }
}

void server::await()
{
    std::mutex m;
    {
        std::unique_lock<std::mutex> lk(m);
        impl_->running_cv_.wait(lk, [this]
        { return impl_->daemon_ == nullptr; });
    }
}


uint16_t server::get_port()
{
    return impl_->port_;
}

std::shared_ptr<router> server::create_router(std::string route_base)
{
    std::shared_ptr<router> r{new router{route_base}};
    impl_->routers_.emplace_back(r);
    return r;
}

///// options setting

void server::set_option_(debug_output value)
{
    impl_->debug_output_ = value.get();
}

void server::set_option_(use_thread_per_connection value)
{
    impl_->use_thread_per_connection_ = value.get();
    if (impl_->use_epoll_if_available_)
    {
        LOG_ERROR(
                "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_epoll_if_available");
        impl_->use_epoll_if_available_ = false; //not compatible!
    }
}

void server::set_option_(use_epoll_if_available value)
{
    impl_->use_epoll_if_available_ = value.get();
    if (impl_->use_thread_per_connection_)
    {
        LOG_ERROR(
                "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_thread_per_connection");
        impl_->use_thread_per_connection_ = false; //not compatible!
    }
}

void server::set_option_(accept_policy_cb value)
{
    impl_->accept_policy_callback_ = value;
}

void server::set_option_(connection_memory_limit value)
{
    //this is a narrowing cast, so ugly! What to do, though?
    impl_->options_.push_back({MHD_OPTION_CONNECTION_MEMORY_LIMIT, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(connection_limit value)
{
    impl_->options_.push_back({MHD_OPTION_CONNECTION_LIMIT, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(connection_timeout value)
{
    impl_->options_.push_back({MHD_OPTION_CONNECTION_TIMEOUT, static_cast<intptr_t>(value.get()), NULL});
}

//void server::set_option_(notify_completed value)
//{
//    //TODO
//}

void server::set_option_(per_ip_connection_limit value)
{
    impl_->options_.push_back({MHD_OPTION_PER_IP_CONNECTION_LIMIT, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(const sockaddr_ptr value)
{
    //why are we casting away the constness? Because MHD isn'T going to modify this, and I want the caller
    // to be assured of this fact.
    impl_->options_.push_back({MHD_OPTION_SOCK_ADDR, 0, const_cast<sockaddr *>(value)});
}

//void server::set_option_(uri_log_callback value)
//{
//    impl_->options_.push_back({MHD_OPTION_URI_LOG_CALLBACK, value, NULL});
//}

void server::set_option_(const server::https_mem_key &value)
{
    // we must make a durable copy of these strings before tossing around char pointers to their internals
    impl_->https_mem_key_.emplace_back(value.get());
    impl_->options_.push_back({MHD_OPTION_HTTPS_MEM_KEY, 0,
                               const_cast<char *>(impl_->https_mem_key_.back().c_str())});
    impl_->ssl_mem_key_set_ = true;
}

void server::set_option_(const server::https_mem_cert &value)
{
    impl_->https_mem_cert_.emplace_back(value.get());
    impl_->options_.push_back({MHD_OPTION_HTTPS_MEM_CERT, 0,
                               const_cast<char *>(impl_->https_mem_cert_.back().c_str())});
    impl_->ssl_mem_cert_set_ = true;
}

//void server::set_option_(https_cred_type value)
//{
//    //TODO
//}

void server::set_option_(const server::https_priorities &value)
{
    impl_->https_priorities_.emplace_back(value.get());
    impl_->options_.push_back({MHD_OPTION_HTTPS_PRIORITIES, 0,
                               const_cast<char *>(impl_->https_priorities_.back().c_str())});
}

void server::set_option_(listen_socket value)
{
    impl_->options_.push_back({MHD_OPTION_LISTEN_SOCKET, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(thread_pool_size value)
{
    impl_->options_.push_back({MHD_OPTION_THREAD_POOL_SIZE, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(unescaper_cb value)
{
    impl_->unescaper_callback_ = value;
    impl_->options_.push_back({MHD_OPTION_UNESCAPE_CALLBACK, (intptr_t) &(impl_->unescaper_callback_shim_), impl_.get()});
}

//void server::set_option_(digest_auth_random value)
//{
//    //TODO
//}

void server::set_option_(nonce_nc_size value)
{
    impl_->options_.push_back({MHD_OPTION_NONCE_NC_SIZE, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(thread_stack_size value)
{
    impl_->options_.push_back({MHD_OPTION_THREAD_STACK_SIZE, static_cast<intptr_t>(value.get()), NULL});
}

void server::set_option_(const server::https_mem_trust &value)
{
    impl_->https_mem_trust_.emplace_back(value.get());
    impl_->options_.push_back({MHD_OPTION_HTTPS_MEM_TRUST, 0,
                               const_cast<char *>(impl_->https_mem_trust_.back().c_str())});
}

void server::set_option_(connection_memory_increment value)
{
    impl_->options_.push_back({MHD_OPTION_CONNECTION_MEMORY_INCREMENT, static_cast<intptr_t>(value.get()), NULL});
}

//void server::set_option_(https_cert_callback value)
//{
//    //TODO
//}

//void server::set_option_(tcp_fastopen_queue_size value)
//{
//    impl_->options_.push_back({MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE, value, NULL});
//}

void server::set_option_(const server::https_mem_dhparams &value)
{
    impl_->https_mem_dhparams_.emplace_back(value.get());
    impl_->options_.push_back({MHD_OPTION_HTTPS_MEM_DHPARAMS, 0,
                               const_cast<char *>(impl_->https_mem_dhparams_.back().c_str())});
}

//void server::set_option_(listening_address_reuse value)
//{
//    impl_->options_.push_back({MHD_OPTION_LISTENING_ADDRESS_REUSE, value, NULL});
//}

void server::set_option_(const server::https_key_password &value)
{
    impl_->https_key_password_.emplace_back(value.get());
    impl_->options_.push_back({MHD_OPTION_HTTPS_KEY_PASSWORD, 0,
                               const_cast<char *>(impl_->https_key_password_.back().c_str())});
}

//void server::set_option_(notify_connection value)
//{
//    //TODO
//}

void server::set_option_(const server::server_identifier &value)
{
    impl_->response_renderer_.set_option(value);
    impl_->server_name_ = value.get().substr(0, value.get().find("/"));
}

void server::set_option_(const server::server_identifier_and_version &value)
{
    impl_->response_renderer_.set_option(value);
    impl_->server_name_ = value.first;
}

void server::set_option_(const server::append_to_server_identifier &value)
{
    impl_->response_renderer_.set_option(value);
}

void server::set_option_(enable_internal_file_cache value)
{
    impl_->response_renderer_.set_option(value);
}

void server::set_option_(internal_file_cache_keep_alive value)
{
    impl_->response_renderer_.set_option(value);
}

}
