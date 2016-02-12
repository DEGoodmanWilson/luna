//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#pragma once

#include <luna/types.h>
#include <sys/socket.h>
#include <string>
#include <regex>
#include <memory>
#include <map>
#include <functional>

namespace luna
{




class server
{
public:

    // configuration parameters
    MAKE_UINT16_T_LIKE(port);
    MAKE_STRING_LIKE(mime_type);

    using access_policy_cb = std::function<bool(const struct sockaddr *, socklen_t )>;


    using endpoint_handler_cb = std::function<status_code(std::vector<std::string> matches,
                                                          query_params params,
                                                          response &body_out)>;

    using error_handler_cb = std::function<response(uint16_t error_code,
                                                    request_method method,
                                                    const std::string &path)>;

    template<typename ...Os>
    server(Os &&...os)
    {
        initialize_();
        set_option(LUNA_FWD(os)...);
    }

    ~server();


    template<typename T>
    void handle_response(request_method method, T path, endpoint_handler_cb callback)
    {
        handle_response(method, std::regex{std::forward<T>(path)}, callback);
    }

    template
    void handle_response(request_method method, const std::regex &path, endpoint_handler_cb callback);

    template
    void handle_response(request_method method, std::regex &&path, endpoint_handler_cb callback);

    bool start();



    template <typename T>
    void set_option(T&& t) {
        set_option_(LUNA_FWD(t));
    }

    template <typename T, typename... Ts>
    void set_option(T&& t, Ts&&... ts) {
        set_option_(LUNA_FWD(t));
        set_option(LUNA_FWD(ts)...);
    }


private:
    class server_impl;
    struct server_impl_deleter { void operator()(server_impl*) const; };
    std::unique_ptr<server_impl, server_impl_deleter> impl_;


    void initialize_();

    void set_option_(mime_type mime_type);
    void set_option_(error_handler_cb handler);
    void set_option_(server::port port);
    void set_option_(access_policy_cb handler);
};

} //namespace luna
