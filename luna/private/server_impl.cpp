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

#include <arpa/inet.h>
#include "luna/private/server_impl.h"

namespace luna
{

const server::accept_policy_cb default_accept_policy_callback_ = [](const struct sockaddr *addr,
                                                                    socklen_t len) -> bool
{
    return true;
};

server::server_impl::server_impl() :
        debug_output_{false},
        ssl_mem_cert_set_{false},
        ssl_mem_key_set_{false},
        use_thread_per_connection_{false},
        use_epoll_if_available_{false},
        daemon_{nullptr},
        accept_policy_callback_{default_accept_policy_callback_},
        port_{0},
        server_name_{LUNA_NAME}
{ }


struct connection_info_struct
{
    request_method connectiontype;
    query_params post_params;
    std::string body;
    MHD_PostProcessor *postprocessor;

    connection_info_struct(request_method method,
                           struct MHD_Connection *connection,
                           size_t buffer_size,
                           MHD_PostDataIterator iter) :
            connectiontype{method}, postprocessor{nullptr}
    {
        postprocessor = MHD_create_post_processor(connection, buffer_size, iter, this);
    }

    ~connection_info_struct()
    {
        if (postprocessor)
        {
            MHD_destroy_post_processor(postprocessor);
        }
    }
};

request_method method_str_to_enum_(const char *method_str)
{
    if (!std::strcmp(method_str, "GET"))
    {
        return request_method::GET;
    }

    if (!std::strcmp(method_str, "PUT"))
    {
        return request_method::PUT;
    }

    if (!std::strcmp(method_str, "POST"))
    {
        return request_method::POST;
    }

    if (!std::strcmp(method_str, "PATCH"))
    {
        return request_method::PATCH;
    }

    if (!std::strcmp(method_str, "DELETE"))
    {
        return request_method::DELETE;
    }

    if (!std::strcmp(method_str, "OPTIONS"))
    {
        return request_method::OPTIONS;
    }

    return request_method::UNKNOWN;
}

//TODO I hate this.
request_method method_str_to_enum_(const std::string &method_str)
{
    return method_str_to_enum_(method_str.c_str());
}

std::string addr_to_str_(const struct sockaddr *addr)
{
    if (addr)
    {
        char str[INET_ADDRSTRLEN];

        switch (addr->sa_family)
        {
            case AF_INET:
                inet_ntop(addr->sa_family,
                          &(reinterpret_cast<const sockaddr_in *>(addr)->sin_addr),
                          str,
                          INET_ADDRSTRLEN);
                break;
            case AF_INET6:
                inet_ntop(addr->sa_family,
                          &(reinterpret_cast<const sockaddr_in6 *>(addr)->sin6_addr),
                          str,
                          INET_ADDRSTRLEN);
                break;
            default:
                return "";
        }
        return std::string{str};
    }
    return "";
}

MHD_ValueKind method_to_value_kind_enum_(request_method method)
{
    if (method == request_method::GET)
    {
        return MHD_GET_ARGUMENT_KIND;
    }

    return MHD_POSTDATA_KIND;
}


int parse_kv_(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    switch (kind)
    {
        case MHD_HEADER_KIND:
        case MHD_RESPONSE_HEADER_KIND:
        {
            auto kv = static_cast<case_insensitive_map *>(cls);
            (*kv)[key] = value ? value : "";
        }
            break;
        default:
        {
            auto kv = static_cast<case_sensitive_map *>(cls);
            (*kv)[key] = value ? value : "";
        }
    }
    return MHD_YES;
}

int server::server_impl::access_handler_callback_(struct MHD_Connection *connection,
                                                  const char *url,
                                                  const char *method_char,
                                                  const char *version,
                                                  const char *upload_data,
                                                  size_t *upload_data_size,
                                                  void **con_cls)
{
    auto start = std::chrono::system_clock::now();

    std::string http_version{version};

    request_method method = method_str_to_enum_(method_char);
    std::string method_str{method_char};

    std::string url_str{url};

    if (!*con_cls)
    {
        connection_info_struct *con_info = new(std::nothrow) connection_info_struct(method,
                                                                                    connection,
                                                                                    65535,
                                                                                    iterate_postdata_shim_);
        if (!con_info) return MHD_NO; //TODO what does this mean?

        *con_cls = con_info;

        return MHD_YES;
    }

    //parse the query params:
    luna::headers header;

    MHD_get_connection_values(connection, MHD_HEADER_KIND, &parse_kv_, &header);

    //find the route, and hit the right callback
    query_params query_params;

    //Query params handling
    MHD_get_connection_values(connection, method_to_value_kind_enum_(method), &parse_kv_, &query_params);

    //POST data handling. This is a tortured flow, and not really MHD' high point.
    auto con_info = static_cast<connection_info_struct *>(*con_cls);
    if (*upload_data_size != 0)
    {
        //TODO note that we just drop BINARY data on the floor at present!! See iterate_postdata_shim_()
        if (MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size) == MHD_NO)
        {
            //MHD couldn't parse it, maybe we can.
            con_info->body.append(upload_data, *upload_data_size);
        }

        *upload_data_size = 0; //flags that we processed everything. This is a funny place to put it.
        return MHD_YES;
    }

    if (!con_info->post_params.empty())//we're done getting postdata, and we have some query params to handle, do something with it
    {
        //if we have post_params, then MHD has ignored the query params. So just overwrite it.
        std::swap(query_params, con_info->post_params);
    }

    // construct request object
    auto ip_address = addr_to_str_(MHD_get_connection_info(connection,
                                                           MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr);

    luna::request request{start, start, ip_address, method, url_str, http_version, {}, query_params, header,
                          con_info->body};

    error_log(log_level::DEBUG, std::string{"Received request for "} + method_str + " " + url_str);



    //iterate through the handlers. Could stand being parallelized, I suppose?
    OPT_NS::optional<response> response;

    std::unique_lock<std::mutex> ulock{lock_};
    for (auto &router : routers_)
    {
        response = router->process_request(request);
        if(response)
        {
            break;
        }
    }
    ulock.unlock();

    if (!response)
    {
        // if there was no response generated by a request handler, make us a 404.
        response = luna::response{404, "Not found"};
    }

    // TODO this is the point where we will want to include middlewares in the future.

    auto response_mhd = response_renderer_.render(request, *response);
    auto retval = MHD_queue_response(connection, response_mhd->status_code, response_mhd->mhd_response);

    request.end = std::chrono::system_clock::now();

    // log it
    auto end_c = std::chrono::system_clock::to_time_t(request.end);
    auto tm = luna::gmtime(end_c);
    access_log(request, *response);

    return retval;
}

/////////// callback shims

int server::server_impl::access_handler_callback_shim_(void *cls,
                                                       struct MHD_Connection *connection,
                                                       const char *url,
                                                       const char *method,
                                                       const char *version,
                                                       const char *upload_data,
                                                       size_t *upload_data_size,
                                                       void **con_cls)
{
    if (!cls) return MHD_NO;

    return static_cast<server_impl *>(cls)->access_handler_callback_(connection,
                                                                     url,
                                                                     method,
                                                                     version,
                                                                     upload_data,
                                                                     upload_data_size,
                                                                     con_cls);
}


int server::server_impl::access_policy_callback_shim_(void *cls, const struct sockaddr *addr, socklen_t addrlen)
{
    if (!cls) return MHD_NO;

    return static_cast<server_impl *>(cls)->accept_policy_callback_(addr, addrlen);
}


void server::server_impl::request_completed_callback_shim_(void *cls, struct MHD_Connection *connection,
                                                           void **con_cls,
                                                           enum MHD_RequestTerminationCode toe)
{
    auto con_info = static_cast<connection_info_struct *>(*con_cls);

    if (con_info && con_info)
    {
        delete con_info;
        *con_cls = NULL;
    }
}

void *server::server_impl::uri_logger_callback_shim_(void *cls, const char *uri, struct MHD_Connection *con)
{
//    LOG_DEBUG(uri); //TODO and stuff about the connection too!
    return nullptr;
}

int server::server_impl::iterate_postdata_shim_(void *cls,
                                                enum MHD_ValueKind kind,
                                                const char *key,
                                                const char *filename,
                                                const char *content_type,
                                                const char *transfer_encoding,
                                                const char *data,
                                                uint64_t off,
                                                size_t size)
{
    auto con_info = static_cast<connection_info_struct *>(cls);
    //TODO this is where we would process binary data. This needs to be implemented
    //TODO unsure how to differentiate between binary (multi-part) post data, and query params, so I am going to wing it
    //  ANnoyingly, when query params are sent here, content_type is nil. As is transfer_encoding. So.


    if (key) //TODO this is a hack, I don't even know if this is a reliable way to detect query params
    {
        auto con_info = static_cast<connection_info_struct *>(cls);
        parse_kv_(&con_info->post_params, kind, key, data);
        return MHD_YES;
    }

    return MHD_YES;
}

void server::server_impl::logger_callback_shim_(void *cls, const char *fm, va_list ap)
{
    //not at all happy with this.
    char message[4096];
    std::vsnprintf(message, sizeof(message), fm, ap);
    error_log(log_level::DEBUG, message);
}

size_t server::server_impl::unescaper_callback_shim_(void *cls, struct MHD_Connection *c, char *s)
{
    auto this_ptr = static_cast<server_impl *>(cls);
    if (this_ptr && this_ptr->unescaper_callback_)
    {
        auto result = this_ptr->unescaper_callback_(s);
        auto old_len = strlen(s);
        memcpy(s, result.c_str(), old_len);
        return (old_len > result.length()) ? result.length() : old_len;
    }

    return strlen(s); //no change
}

//void server::server_impl::notify_connection_callback_shim_(void *cls,
//                                                           struct MHD_Connection *connection,
//                                                           void **socket_context,
//                                                           enum MHD_ConnectionNotificationCode toe)
//{
//    auto this_ptr = static_cast<server_impl *>(cls);
//    if (this_ptr && this_ptr->notify_connection_callback_)
//    {
//        return this_ptr->notify_connection_callback_(connection, socket_context, toe);
//    }
//}

} //namespace luna
