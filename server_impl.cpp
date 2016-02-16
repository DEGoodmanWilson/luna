//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#include "server_impl.h"
#include <stdarg.h>

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
    //TODO NEED TO SET UP MORE OPTIONS!!!!!!!!!
    // Would strongly prefer if the wrapper constructor could just forward all its varargs to this constructor
    daemon_ = MHD_start_daemon(MHD_USE_POLL_INTERNALLY,
                               port_,
                               access_policy_callback_shim_,
                               this,
                               access_handler_callback_shim_,
                               this,
                               MHD_OPTION_THREAD_POOL_SIZE,
                               10, //use 10 threads
                               MHD_OPTION_END);

    //TODO check if daemon_ is null. It should not be null.
    if (!daemon_)
    {
        std::cout << "Daemon failed to start" << std::endl;
    }
    //TODO better logging facilities than cout
    std::cout << "New server on port " << port_ << std::endl;
}

bool server::server_impl::is_running()
{
    return (!!daemon_);
}

void server::server_impl::stop()
{
    if (daemon_)
    {
        MHD_stop_daemon(daemon_);
        daemon_ = nullptr;
    }
}

server::server_impl::~server_impl()
{
    stop();
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

    return static_cast<server_impl *>(cls)->access_policy_handler_(addr, addrlen);
}


void server::server_impl::request_completed_callback_shim_(void *cls, struct MHD_Connection *connection,
                                                           void **con_cls,
                                                           enum MHD_RequestTerminationCode toe)
{
    auto this = static_cast<server_impl*>(cls);
    if(this && this->request_completed_callback_)
    {
        return this->request_completed_callback_(connection, cons_cls, toe);
    }
}

void server::server_impl::uri_logger_callback_shim_(void *cls, const char *uri, struct MHD_Connection *con)
{
    auto this = static_cast<server_impl*>(cls);
    if(this && this->logger_callback_)
    {
        return this->logger_callback_(uri); //TODO and stuff about the connection too!
    }
}

//http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
std::string string_format(const std::string fmt_str, va_list ap)
{
    int final_n, n = ((int) fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    while (1)
    {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
        {
            n += abs(final_n - n + 1);
        }
        else
        {
            break;
        }
    }
    return std::string(formatted.get());
}

void server::server_impl::logger_callback_shim_(void *cls, const char *fm, va_list ap)
{
    auto this = static_cast<server_impl*>(cls);
    if(this && this->logger_callback_)
    {
        return this->logger_callback_(string_format(fm, ap));
    }
}

size_t server::server_impl::unescaper_callback_shim_(void *cls, struct MHD_Connection *c, char *s)
{
    auto this = static_cast<server_impl*>(cls);
    if(this && this->unescaper_callback_)
    {
        return this->unescaper_callback_(c, s);
    }
}

void server::server_impl::notify_connection_callback_shim_(void *cls,
                                                           struct MHD_Connection *connection,
                                                           void **socket_context,
                                                           enum MHD_ConnectionNotificationCode toe)
{
    auto this = static_cast<server_impl*>(cls);
    if(this && this->notify_connection_callback_)
    {
        return this->notify_connection_callback_(connection, socket_context, toe);
    }
}



///// options setting

void server::server_impl::set_option(const server::mime_type &mime_type)
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

void server::server_impl::set_option(server::connection_memory_limit value)
{
    //this is a narrowing cast, so ugly! What to do, though?
    options_.push_back({MHD_OPTION_CONNECTION_MEMORY_LIMIT, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option(server::connection_limit value)
{
    options_.push_back({MHD_OPTION_CONNECTION_LIMIT, value, NULL});
}

void server::server_impl::set_option(server::connection_timeout value)
{
    options_.push_back({MHD_OPTION_CONNECTION_TIMEOUT, value, NULL});
}

//void server::server_impl::set_option(server::notify_completed value)
//{
//    //TODO
//}

void server::server_impl::set_option(server::per_ip_connection_limit value)
{
    options_.push_back({MHD_OPTION_PER_IP_CONNECTION_LIMIT, value, NULL});
}

void server::server_impl::set_option(const sockaddr *value)
{
    //why are we casting away the constness? Because MHD isn'T going to modify this, and I want the caller
    // to be assured of this fact.
    options_.push_back({MHD_OPTION_SOCK_ADDR, 0, const_cast<sockaddr*>(value)});
}

//void server::server_impl::set_option(server::uri_log_callback value)
//{
//    options_.push_back({MHD_OPTION_URI_LOG_CALLBACK, value, NULL});
//}

void server::server_impl::set_option(const server::https_mem_key& value)
{
    //TODO this feel very dodgy to me. But I can't quite put my finger on the case where this pointer becomes prematurely invalid
    options_.push_back({MHD_OPTION_HTTPS_MEM_KEY, 0, const_cast<char*>(value.c_str())});
}

void server::server_impl::set_option(const server::https_mem_cert& value)
{
    options_.push_back({MHD_OPTION_HTTPS_MEM_CERT, 0, const_cast<char*>(value.c_str())});
}

//void server::server_impl::set_option(server::https_cred_type value)
//{
//    //TODO
//}

void server::server_impl::set_option(const server::https_priorities& value)
{
    options_.push_back({MHD_OPTION_HTTPS_PRIORITIES, 0, const_cast<char*>(value.c_str())});
}

void server::server_impl::set_option(server::listen_socket value)
{
    options_.push_back({MHD_OPTION_LISTEN_SOCKET, value, NULL});
}

//void server::server_impl::set_option(server::external_logger value)
//{
//    //TODO
//}

void server::server_impl::set_option(server::thread_pool_size value)
{
    options_.push_back({MHD_OPTION_THREAD_POOL_SIZE, value, NULL});
}

//void server::server_impl::set_option(server::unescape_callback value)
//{
//    //TODO
//}

//void server::server_impl::set_option(server::digest_auth_random value)
//{
//    //TODO
//}

void server::server_impl::set_option(server::nonce_nc_size value)
{
    options_.push_back({MHD_OPTION_NONCE_NC_SIZE, value, NULL});
}

void server::server_impl::set_option(server::thread_stack_size value)
{
    options_.push_back({MHD_OPTION_THREAD_STACK_SIZE, static_cast<intptr_t>(value), NULL});
}

void server::server_impl::set_option(const server::https_mem_trust &value)
{
    options_.push_back({MHD_OPTION_HTTPS_MEM_TRUST, 0, const_cast<char*>(value.c_str())});
}

void server::server_impl::set_option(server::connection_memory_increment value)
{
    options_.push_back({MHD_OPTION_CONNECTION_MEMORY_INCREMENT, static_cast<intptr_t>(value), NULL});
}

//void server::server_impl::set_option(server::https_cert_callback value)
//{
//    //TODO
//}

void server::server_impl::set_option(server::tcp_fastopen_queue_size value)
{
    options_.push_back({MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE, value, NULL});
}

void server::server_impl::set_option(const server::https_mem_dhparams &value)
{
    options_.push_back({MHD_OPTION_HTTPS_MEM_DHPARAMS, 0, const_cast<char*>(value.c_str())});
}

void server::server_impl::set_option(server::listening_address_reuse value)
{
    options_.push_back({MHD_OPTION_LISTENING_ADDRESS_REUSE, value, NULL});
}

void server::server_impl::set_option(const server::https_key_password &value)
{
    options_.push_back({MHD_OPTION_HTTPS_KEY_PASSWORD, 0, const_cast<char*>(value.c_str())});
}

//void server::server_impl::set_option(server::notify_connection value)
//{
//    //TODO
//}


} //namespace luna