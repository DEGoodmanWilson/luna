//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#include "server_impl.h"

namespace luna
{

std::string default_mime_type{"text/html"};

static const bool is_error_(status_code code)
{
    if ((code >= 200) && (code < 300)) return false;

    return true;
}


request_method method_str_to_enum(const char *method_str)
{
    if (!std::strcmp(method_str, GET))
    {
        return request_method::GET;
    }

    if (!std::strcmp(method_str, PUT))
    {
        return request_method::PUT;
    }

    if (!std::strcmp(method_str, POST))
    {
        return request_method::POST;
    }

    if (!std::strcmp(method_str, PATCH))
    {
        return request_method::PATCH;
    }

    if (!std::strcmp(method_str, DELETE))
    {
        return request_method::DELETE;
    }

    return request_method::UNKNOWN;
}


server::server_impl::server_impl() :
        error_handler_{default_error_handler_}, access_policy_handler_{default_access_policy_handler_}, daemon_{}
{ }


void server::server_impl::start()
{
    //TODO not super happy that this has to come outside the constructor.
    // Would strongly prefer if the wrapper constructor could just forward all its varargs to this constructor
    daemon_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                     port_,
                     access_policy_callback_shim_,
                     this,
                     access_handler_callback_shim_,
                     this,
                     MHD_OPTION_END);
    //TODO check if daemon_ is null. It should not be null.
    //TODO better logging facilities than cout
    std::cout << "New server on port " << port_ << std::endl;
}

server::server_impl::~server_impl()
{
    if (daemon_)
    {
        MHD_stop_daemon(daemon_);
    }
}


void server::server_impl::handle_response(request_method method,
                                          const std::regex &path,
                                          server::endpoint_handler_cb callback)
{
    response_handlers_[method].emplace_back(std::make_pair(path, callback));
}


int server::server_impl::parse_kv_(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    auto kv = static_cast<query_params *>(cls);
    kv->operator[](key) = value;
    return MHD_YES;
}

int server::server_impl::access_handler_callback_(struct MHD_Connection *connection,
                                                  const char *url,
                                                  const char *method_str,
                                                  const char *version,
                                                  const char *upload_data,
                                                  size_t *upload_data_size,
                                                  void **con_cls)
{
    //TODO
    // if(!con_cls) return 0; //TODO WTF

    //parse the query params:
    std::map<std::string, std::string> header;

    MHD_get_connection_values(connection, MHD_HEADER_KIND, &parse_kv_, &header);


    //find the route, and hit the right callback
    request_method method = method_str_to_enum(method_str);

    query_params query_params;
    MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &parse_kv_, &query_params);

    //iterate through the handlers. Could stand being parallelized, I suppose?
    for (const auto &handler_pair : response_handlers_[method])
    {
        std::smatch pieces_match;
        auto path_regex = std::get<std::regex>(handler_pair);
        auto callback = std::get<endpoint_handler_cb>(handler_pair);
        std::string url_str{url};
        if (std::regex_match(url_str, pieces_match, path_regex))
        {
            std::vector<std::string> matches;
            std::cout << "match: " << url << '\n';
            for (size_t i = 0; i < pieces_match.size(); ++i)
            {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                std::cout << "  submatch " << i << ": " << piece << '\n';
                matches.emplace_back(sub_match.str());
            }

            response response;
            status_code status_code;
            try
            {
                status_code = callback(matches, query_params, response);
            }
            catch (const std::exception &e)
            {
                status_code = 500;
                //TODO render the stack trace, etc.
            }

            if (is_error_(status_code))
            {
                return render_error_(status_code, connection, url, method);
            }

            //terminate on first success
            return render_response_(status_code, response, connection, url, method);
        }
    }

    /* unsupported HTTP method */
    return render_error_(404, connection, url, method);
}


int server::server_impl::render_response_(status_code status_code,
                                          response response,
                                          MHD_Connection *connection,
                                          const char *url,
                                          request_method method) const
{
    auto mhd_response = MHD_create_response_from_buffer(response.content.length(),
                                                        (void *) response.content.c_str(),
                                                        MHD_RESPMEM_MUST_COPY);
    auto ret = MHD_queue_response(connection,
                                  status_code,
                                  mhd_response);
    MHD_add_response_header(mhd_response,
                            MHD_HTTP_HEADER_CONTENT_ENCODING,
                            response.content_type.c_str());
    MHD_destroy_response(mhd_response);
    return ret;
}

int server::server_impl::render_error_(uint16_t error_code,
                                       MHD_Connection *connection,
                                       const char *url,
                                       request_method method) const
{
    struct MHD_Response *response;
    /* unsupported HTTP method */
    auto error_page = error_handler_(error_code, method, url);

    return render_response_(error_code, error_page, connection, url, method);
}


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

    return static_cast<server_impl *>(cls)->access_policy_handler_(addr, addrlen);
}

void server::server_impl::set_option(server::mime_type mime_type)
{
    default_mime_type = mime_type;
}

void server::server_impl::set_option(server::error_handler_cb handler)
{
    error_handler_ = handler;
}

void server::server_impl::set_option(server::port port)
{
    port_ = port;
}

void server::server_impl::set_option(server::access_policy_cb handler)
{
    access_policy_handler_ = handler;
}

} //namespace luna