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

server::~server()
{

}

void server::start()
{
    impl_->start();
}

void server::stop()
{
    impl_->stop();
}

void server::await()
{
    impl_->await();
}


server::port server::get_port()
{
    return impl_->get_port();
}

void server::server_impl_deleter::operator()(server::server_impl *ptr) const
{ delete ptr; }

void server::set_option_(start_on_construction value)
{
    start_on_construct_ = static_cast<bool>(value);
}
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

// middleware
void server::set_option_(middleware::before_request_handler value)
{
    impl_->set_option(value);
}
void server::set_option_(middleware::after_request_handler value)
{
    impl_->set_option(value);
}
void server::set_option_(middleware::after_error value)
{
    impl_->set_option(value);
}

// caching
void server::set_option_(std::pair<cache::read, cache::write> value)
{
    impl_->set_option(value);
}

void server::set_option_(server::enable_internal_file_cache value)
{
    impl_->set_option(value);
}

void server::set_option_(internal_file_cache_keep_alive value)
{
    impl_->set_option(value);
}

server::request_handler_handle server::handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback, parameter::validators &&validations)
{
    return impl_->handle_request(method, std::regex{std::move(path)}, callback, std::move(validations));
}

server::request_handler_handle server::handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback, parameter::validators &&validations)
{
    return impl_->handle_request(method, std::regex{path}, callback, std::move(validations));
}

server::request_handler_handle server::handle_request(request_method method, std::regex &&path, endpoint_handler_cb callback, const parameter::validators &validations)
{
    return impl_->handle_request(method, std::regex{std::move(path)}, callback, validations);
}

server::request_handler_handle server::handle_request(request_method method, const std::regex &path, endpoint_handler_cb callback, const parameter::validators &validations)
{
    return impl_->handle_request(method, std::regex{path}, callback, validations);
}

server::request_handler_handle server::serve_files(const std::string &mount_point, const std::string &path_to_files)
{
    return impl_->serve_files(std::move(mount_point), std::move(path_to_files));
}

server::request_handler_handle server::serve_files(std::string &&mount_point, std::string &&path_to_files)
{
    return impl_->serve_files(std::move(mount_point), std::move(path_to_files));
}

server::error_handler_handle server::handle_404(error_handler_cb callback)
{
    return impl_->handle_404(callback);
}

server::error_handler_handle server::handle_error(status_code code, error_handler_cb callback)
{
    return impl_->handle_error(code, callback);
}

void server::remove_request_handler(server::request_handler_handle item)
{
    impl_->remove_request_handler(item);
}

void server::remove_error_handler(server::error_handler_handle item)
{
    impl_->remove_error_handler(item);
}

void server::add_global_header(std::string &&header, std::string &&value)
{
    impl_->add_global_header(std::move(header), std::move(value));
}

void server::add_global_header(const std::string &header, std::string &&value)
{
    impl_->add_global_header(header, std::move(value));
}

void server::add_global_header(std::string &&header, const std::string &value)
{
    impl_->add_global_header(std::move(header), value);
}

void server::add_global_header(const std::string &header, const std::string &value)
{
    impl_->add_global_header(header, value);
}


}
