//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#include "server.h"
#include "server_impl.h"

namespace luna
{


void server::initialize_()
{
    impl_.reset(new server_impl());
}

server::~server()
{

}

void server::server_impl_deleter::operator()(server::server_impl* ptr) const { delete ptr; }


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

void server::handle_response(request_method method, const std::regex &path, endpoint_handler_cb callback)
{
    impl_->handle_response(method, std::regex{path}, callback);
}

void server::handle_response(request_method method, std::regex &&path, endpoint_handler_cb callback)
{
    impl_->handle_response(method, std::regex{std::move(path)}, callback);
}

bool server::start()
{
    return impl_->start();
}

}