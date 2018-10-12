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

#include "router.h"
#include "luna/private/router_impl.h"

namespace luna
{

router::router(std::string route_base) : impl_{std::make_unique<router_impl>(route_base)}
{
}

// move constructor and operator
router::router(router &&) noexcept = default;

router &router::operator=(router &&) noexcept = default;

router::~router() = default;

void router::set_mime_type(std::string mime_type)
{
    impl_->set_mime_type(mime_type);
}

void router::handle_request(request_method method,
                            std::regex route,
                            router::endpoint_handler_cb callback,
                            parameter::validators validations)
{
    impl_->handle_request(method, route, callback, validations);
}

void router::handle_request(request_method method,
                            std::string route,
                            router::endpoint_handler_cb callback,
                            parameter::validators validations)
{
    impl_->handle_request(method, route, callback, validations);
}

void router::serve_files(std::string mount_point, std::string path_to_files)
{
    impl_->serve_files(mount_point, path_to_files);
}

void router::add_header(std::string &&key, std::string &&value)
{
    impl_->add_header(std::move(key), std::move(value));
}

OPT_NS::optional<luna::response> router::process_request(request &request)
{
    return impl_->process_request(request);
}

} //namespace luna
