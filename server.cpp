//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#include "server.h"
#include "server_impl.h"

namespace luna
{

server::server(uint16_t port) : impl_{std::make_unique<server_impl>(server::port{port})}
{ }

server::~server()
{ }


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


void server::set_error_handler(server::error_handler_cb handler)
{
    impl_->set_error_handler(handler);
}

}