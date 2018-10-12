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

server &server::operator=(server &&) = default;


// TODO
// * multiple headers with same name
// * catch exceptions in user logger and middleware functions, throw 500 when they happen.

///////////////////////////




bool server::start(uint16_t port)
{
    return impl_->start(port);
}

bool server::start_async(uint16_t port)
{
    return impl_->start_async(port);
}

bool server::is_running()
{
    return impl_->is_running();
}

void server::stop()
{
    impl_->stop();
}

void server::await()
{
    impl_->await();
}

uint16_t server::get_port()
{
    return impl_->get_port();
}

std::shared_ptr<router> server::create_router(std::string route_base)
{
    return impl_->create_router(route_base);
}

///// options setting

void server::set_option_(debug_output value)
{
    impl_->set_option_(value);
}

void server::set_option_(use_thread_per_connection value)
{
    impl_->set_option_(value);
}

void server::set_option_(use_epoll_if_available value)
{
    impl_->set_option_(value);
}

void server::set_option_(accept_policy_cb value)
{
    impl_->set_option_(value);
}

void server::set_option_(connection_memory_limit value)
{
    impl_->set_option_(value);
}

void server::set_option_(connection_limit value)
{
    impl_->set_option_(value);
}

void server::set_option_(connection_timeout value)
{
    impl_->set_option_(value);
}

//void server::set_option_(notify_completed value)
//{
//TODO
//impl_->set_option_(value);
//}

void server::set_option_(per_ip_connection_limit value)
{
    impl_->set_option_(value);
}

void server::set_option_(const sockaddr_ptr value)
{
    impl_->set_option_(value);
}

//void server::set_option_(uri_log_callback value)
//{
//impl_->set_option_(value);
//}

void server::set_option_(const server::https_mem_key &value)
{
    impl_->set_option_(value);
}

void server::set_option_(const server::https_mem_cert &value)
{
    impl_->set_option_(value);
}

//void server::set_option_(https_cred_type value)
//{
//    //TODO
//impl_->set_option_(value);
//}

void server::set_option_(const server::https_priorities &value)
{
    impl_->set_option_(value);}

void server::set_option_(listen_socket value)
{
    impl_->set_option_(value);
}

void server::set_option_(thread_pool_size value)
{
    impl_->set_option_(value);
}

void server::set_option_(unescaper_cb value)
{
    impl_->set_option_(value);
}

//void server::set_option_(digest_auth_random value)
//{
//    //TODO
//impl_->set_option_(value);
//}

void server::set_option_(nonce_nc_size value)
{
    impl_->set_option_(value);
}

void server::set_option_(thread_stack_size value)
{
    impl_->set_option_(value);
}

void server::set_option_(const server::https_mem_trust &value)
{
    impl_->set_option_(value);
}

void server::set_option_(connection_memory_increment value)
{
    impl_->set_option_(value);
}

//void server::set_option_(https_cert_callback value)
//{
//    //TODO
//impl_->set_option_(value);
//}

//void server::set_option_(tcp_fastopen_queue_size value)
//{
//impl_->set_option_(value);
//}

void server::set_option_(const server::https_mem_dhparams &value)
{
    impl_->set_option_(value);
}

//void server::set_option_(listening_address_reuse value)
//{
//impl_->set_option_(value);
//}

void server::set_option_(const server::https_key_password &value)
{
    impl_->set_option_(value);
}

//void server::set_option_(notify_connection value)
//{
//    //TODO
//impl_->set_option_(value);
//}

void server::set_option_(const server::server_identifier &value)
{
    impl_->set_option_(value);
}

void server::set_option_(const server::server_identifier_and_version &value)
{
    impl_->set_option_(value);
}

void server::set_option_(const server::append_to_server_identifier &value)
{
    impl_->set_option_(value);
}

void server::set_option_(enable_internal_file_cache value)
{
    impl_->set_option_(value);
}

void server::set_option_(internal_file_cache_keep_alive value)
{
    impl_->set_option_(value);
}

} // namespace luna
