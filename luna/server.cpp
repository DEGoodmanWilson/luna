//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
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

void server::start_()
{
    impl_->start();
}

server::~server()
{

}


server::port server::get_port()
{
    return impl_->get_port();
}

void server::server_impl_deleter::operator()(server::server_impl *ptr) const
{ delete ptr; }


void server::set_option_(use_ssl value)
{
    impl_->set_option(value);
}

void server::set_option_(use_thread_per_connection value)
{
    impl_->set_option(value);
}

void server::set_option_(mime_type mime_type)
{
    impl_->set_option(mime_type);
}

void server::set_option_(error_handler_cb handler)
{
    impl_->set_option(handler);
}

void server::set_option_(port port)
{
    impl_->set_option(port);
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

//void server::set_option_(const https_mem_dhparams &value)
//{
//    impl_->set_option(value);
//}

//void server::set_option_(listening_address_reuse value)
//{
//    impl_->set_option(value);
//}

//void server::set_option_(const https_key_password &value)
//{
//    impl_->set_option(value);
//}

//void server::set_option_(notify_connection value)
//{
//    impl_->set_option(value);
//}

server::request_handler_handle server::handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback)
{
    return impl_->handle_request(method, std::regex{std::move(path)}, callback);
}

server::request_handler_handle server::handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback)
{
    return impl_->handle_request(method, std::regex{path}, callback);
}

void server::remove_request_handler(server::request_handler_handle item)
{
    impl_->remove_request_handler(item);
}

}
