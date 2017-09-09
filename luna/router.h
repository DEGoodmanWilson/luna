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

#pragma once

#include <luna/types.h>
#include <luna/config.h>
#include <luna/optional.hpp>
#include <regex>

#include <iostream>

namespace luna
{

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

class router
{
public:

    template<typename R>
    router(R &&route_base) : route_base_{std::forward<std::string>(route_base)}
    {
        //remove trailing slashes
        if(route_base_.back() == '/')
        {
            route_base_.pop_back();
        }
        std::cout << route_base_ << std::endl;
    }

    router(const router &r) : route_base_{r.route_base_}, request_handlers_{r.request_handlers_}, error_handlers_{r.error_handlers_}, headers_{r.headers_}
    {}

    router(router &&r) : route_base_{std::move(r.route_base_)}, request_handlers_{std::move(r.request_handlers_)}, error_handlers_{std::move(r.error_handlers_)}, headers_{std::move(r.headers_)}
    {}

    using endpoint_handler_cb = std::function<response(const request &req)>;

    using error_handler_cb = std::function<void( const request &request, response
    &response)>;
    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    using request_handler_handle = std::pair<request_method, request_handlers::const_iterator>;
    using error_handler_handle = status_code;

    template<typename P>
    request_handler_handle handle_request(request_method method,
                                          P &&path,
                                          endpoint_handler_cb callback)
    {
        std::lock_guard<std::mutex> guard{lock_};
        auto retval = std::make_pair(method,
                              request_handlers_[method].insert(std::end(request_handlers_[method]),
                                                               std::make_tuple(std::regex{std::forward<P>(path)},
                                                                               callback,
                                                                               luna::parameter::validators{})));
        std::cout << request_handlers_[method].size() << std::endl;
        return retval;
    }

    template<typename P, typename V>
    request_handler_handle handle_request(request_method method,
                                          P &&path,
                                          endpoint_handler_cb callback,
                                          V &&validations)
    {
        std::lock_guard<std::mutex> guard{lock_};
        return std::make_pair(method,
                              request_handlers_[method].insert(std::end(request_handlers_[method]),
                                                               std::make_tuple(std::regex{std::forward<P>(path)},
                                                                               callback,
                                                                               std::forward<V>(validations))));
    }

    template<typename M, typename P>
    request_handler_handle serve_files(M &&mount_point, P &&path_to_files)
    {
        std::regex regex{std::forward<std::string>(mount_point) + "(.*)"};
        std::string local_path{std::forward<std::string>(path_to_files) + "/"};
        return handle_request(request_method::GET, regex, [=](const request &req) -> response
        {
            std::string path = local_path + req.matches[1];

            LOG_DEBUG(std::string{"File requested:  "} + req.matches[1]);
            LOG_DEBUG(std::string{"Serve from    :  "} + path);

            return response::from_file(path);
        });
    }

    void remove_request_handler(request_handler_handle item);


    // a shortcut for writing 404 handlers.
    error_handler_handle handle_404(error_handler_cb callback);

    error_handler_handle handle_error(status_code code, error_handler_cb callback);

    void remove_error_handler(error_handler_handle item);

    template<typename H, typename V>
    void add_global_header(H &&header, V &&value)
    {
        headers_[header] = value;
    };


    // for use by the Server object
    std::experimental::optional<luna::response> process_request(request &request); //not const because of lock_

private:
    std::string route_base_;
    std::mutex lock_;
    std::map<request_method, request_handlers> request_handlers_;
    std::map<status_code, error_handler_cb> error_handlers_;
    headers headers_;
};


} //namespace luna