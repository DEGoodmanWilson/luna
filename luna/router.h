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

    router() // use default empty string for route_base_, same as specifying "/"
    {}

    template<typename R>
    router(R &&route_base) : route_base_{std::forward<std::string>(route_base)}
    {
        //remove trailing slashes
        if (route_base_.back() == '/')
        {
            route_base_.pop_back();
        }
        std::cout << route_base_ << std::endl;
    }

    router(const router &r) : route_base_{r.route_base_}, request_handlers_{r.request_handlers_}, headers_{r.headers_}
    {}

    router(router &&r) :
            route_base_{std::move(r.route_base_)},
            request_handlers_{std::move(r.request_handlers_)},
            headers_{std::move(r.headers_)}
    {}

    using endpoint_handler_cb = std::function<

    response(const request &req)

    >;

    template<typename P>
    void handle_request(request_method method,
                        P &&path,
                        endpoint_handler_cb callback)
    {
        std::lock_guard<std::mutex> guard{lock_};
        request_handlers_[method].insert(std::end(request_handlers_[method]),
                                         std::make_tuple(std::regex{std::forward<P>(path)},
                                                         callback,
                                                         luna::parameter::validators{}));
    }

    template<typename P, typename V>
    void handle_request(request_method method,
                        P &&path,
                        endpoint_handler_cb callback,
                        V &&validations)
    {
        std::lock_guard<std::mutex> guard{lock_};
        request_handlers_[method].insert(std::end(request_handlers_[method]),
                                         std::make_tuple(std::regex{std::forward<P>(path)},
                                                         callback,
                                                         std::forward<V>(validations)));
    }

    template<typename M, typename P>
    void serve_files(M &&mount_point, P &&path_to_files)
    {
        std::regex regex{std::forward<std::string>(mount_point) + "(.*)"};
        std::string local_path{std::forward<std::string>(path_to_files) + "/"};
        handle_request(request_method::GET, regex, [=](const request &req) -> response
        {
            std::string path = local_path + req.matches[1];

            LOG_DEBUG(std::string{"File requested:  "} + req.matches[1]);
            LOG_DEBUG(std::string{"Serve from    :  "} + path);

            return response::from_file(path);
        });
    }

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
    using request_handlers = std::vector<std::tuple<std::regex, endpoint_handler_cb, parameter::validators>>;
    std::map<request_method, request_handlers> request_handlers_;
    headers headers_;
};


} //namespace luna