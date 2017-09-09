//
//      _
//  ___/__)
// (, /      __   _
//   /   (_(_/ (_(_(_
//  (________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

#include "server.h"
#include "server_impl.h"

namespace luna
{


server::operator bool()
{
    return impl_->is_running();
}

void server::initialize_()
{
    impl_.reset(new server_impl());
}

server::~server()
{

}

bool server::start(uint16_t port)
{
    return impl_->start(port);
}

bool server::start_async(uint16_t port)
{
    return impl_->start_async(port);
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

void server::server_impl_deleter::operator()(server::server_impl *ptr) const
{ delete ptr; }


void server::set_option_(debug_output value)
{
    impl_->set_option(value);
}

void server::set_option_(use_thread_per_connection value)
{
    impl_->set_option(value);
}

void server::set_option_(use_epoll_if_available value)
{
    impl_->set_option(value);
}

void server::set_option_(mime_type mime_type)
{
    impl_->set_option(mime_type);
}

void server::set_option_(accept_policy_cb handler)
{
    impl_->set_option(handler);
}

void server::set_option_(connection_memory_limit value)
{
    impl_->set_option(value);
}

void server::set_option_(connection_limit value)
{
    impl_->set_option(value);
}

void server::set_option_(connection_timeout value)
{
    impl_->set_option(value);
}

void server::set_option_(per_ip_connection_limit value)
{
    impl_->set_option(value);
}

void server::set_option_(const sockaddr_ptr value)
{
    impl_->set_option(value);
}

void server::set_option_(https_mem_key value)
{
    impl_->set_option(value);
}

void server::set_option_(https_mem_cert value)
{
    impl_->set_option(value);
}

//void server::set_option_(https_cred_type value)
//{
//    impl_->set_option(value);
//}

void server::set_option_(https_priorities value)
{
    impl_->set_option(value);
}

void server::set_option_(listen_socket value)
{
    impl_->set_option(value);
}

void server::set_option_(thread_pool_size value)
{
    impl_->set_option(value);
}

void server::set_option_(unescaper_cb value)
{
    impl_->set_option(value);
}

//void server::set_option_(digest_auth_random value)
//{
//    impl_->set_option(value);
//}

void server::set_option_(nonce_nc_size value)
{
    impl_->set_option(value);
}

void server::set_option_(thread_stack_size value)
{
    impl_->set_option(value);
}

void server::set_option_(const https_mem_trust &value)
{
    impl_->set_option(value);
}

void server::set_option_(connection_memory_increment value)
{
    impl_->set_option(value);
}

//void server::set_option_(https_cert_callback value)
//{
//    impl_->set_option(value);
//}

//void server::set_option_(tcp_fastopen_queue_size value)
//{
//    impl_->set_option(value);
//}

void server::set_option_(const https_mem_dhparams &value)
{
    impl_->set_option(value);
}

//void server::set_option_(listening_address_reuse value)
//{
//    impl_->set_option(value);
//}

void server::set_option_(const https_key_password &value)
{
    impl_->set_option(value);
}

//void server::set_option_(notify_connection value)
//{
//    impl_->set_option(value);
//}

void server::set_option_(const server_identifier &value)
{
    impl_->set_option(value);
}

void server::set_option_(const append_to_server_identifier &value)
{
    impl_->set_option(value);
}

// caching
void server::set_option_(server::enable_internal_file_cache value)
{
    impl_->set_option(value);
}

void server::set_option_(internal_file_cache_keep_alive value)
{
    impl_->set_option(value);
}

void server::add_global_header(const std::string &header, const std::string &value)
{
    impl_->add_global_header(header, value);
}
void server::add_global_header(std::string &&header, const std::string &value)
{
    impl_->add_global_header(std::move(header), value);
}
void server::add_global_header(const std::string &header, std::string &&value)
{
    impl_->add_global_header(header, std::move(value));
}
void server::add_global_header(std::string &&header, std::string &&value)
{
    impl_->add_global_header(std::move(header), std::move(value));
}


void server::add_router(const router &router)
{
    impl_->add_router(router);
}

}
