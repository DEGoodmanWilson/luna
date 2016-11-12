//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <magic.h>
#include "luna/private/server_impl.h"
#include "luna/config.h"


namespace luna
{

extern logger_cb logger_;

//TODO do this better. Make this an ostream with a custom function. It's not like we haven't done that before.
#define LOG(level, mesg) if (logger_) \
{ \
    logger_(level, mesg); \
}

#define LOG_FATAL(mesg) if (logger_) \
{ \
    logger_(log_level::FATAL, mesg); \
}

#define LOG_ERROR(mesg) if (logger_) \
{ \
    logger_(log_level::ERROR, mesg); \
}

#define LOG_INFO(mesg) if (logger_) \
{ \
    logger_(log_level::INFO, mesg); \
}

#define LOG_DEBUG(mesg) if (logger_) \
{ \
    logger_(log_level::DEBUG, mesg); \
}


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

std::string default_mime_type{"text/html; charset=UTF-8"};

static const server::error_handler_cb default_error_handler_callback_ = [](response &response,
                                                                           request_method method,
                                                                           const std::string &path)
    {
        if (response.content.empty())
        {
            response.content_type = "text/html; charset=UTF-8";
            //we'd best render it ourselves.
            switch (response.status_code)
            {
                case 404:
                    response.content = "<h1>Not found</h1>";
                    break;
                default:
                    response.content = "<h1>So sorry, generic server error</h1>";
            }
        }
    };

static const server::accept_policy_cb default_accept_policy_callback_ = [](const struct sockaddr *addr,
                                                                           socklen_t len) -> bool
    {
        return true;
    };

static status_code default_success_code_(request_method method)
{
    if (method == request_method::POST)
    {
        return 201;
    }

    return 200;
}

static bool is_error_(status_code code)
{
    if (code < 300) return false;

    return true;
}

static bool is_redirect_(status_code code)
{
    if ((code >= 300) && code < 400) return true;

    return false;
}

static request_method method_str_to_enum_(const char *method_str)
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

    if (!std::strcmp(method_str, OPTIONS))
    {
        return request_method::OPTIONS;
    }

    return request_method::UNKNOWN;
}

//TODO I hate this.
static request_method method_str_to_enum_(const std::string &method_str)
{
    return method_str_to_enum_(method_str.c_str());
}

static std::string addr_to_str_(const struct sockaddr *addr)
{
    //TODO do this better. This is stupid.
    if(addr->sa_family == 0x02)
        return std::to_string(addr->sa_data[2])+"."+std::to_string(addr->sa_data[3])+"."+std::to_string(addr->sa_data[4])+"."+std::to_string(addr->sa_data[5]);
    return "";
}

static MHD_ValueKind method_to_value_kind_enum_(request_method method)
{
    if (method == request_method::GET)
    {
        return MHD_GET_ARGUMENT_KIND;
    }

    return MHD_POSTDATA_KIND;
}
///////////////////////////

server::server_impl::server_impl() :
        debug_output_{false},
        lock_{},
        ssl_mem_cert_set_{false},
        ssl_mem_key_set_{false},
        use_thread_per_connection_{false},
        use_epoll_if_available_{false},
        daemon_{nullptr},
        error_handler_callback_{default_error_handler_callback_},
        accept_policy_callback_{default_accept_policy_callback_},
        port_{8080}
{ }


void server::server_impl::start()
{
    //TODO not super happy that this has to come outside the constructor.
    // Would strongly prefer if the wrapper constructor could just forward all its varargs to this constructor

    MHD_OptionItem options[options_.size() + 1];
    uint16_t idx = 0;
    for (const auto &opt : options_)
    {
        options[idx++] = opt; //copy it in, whee.
    }
    options[idx] = {MHD_OPTION_END, 0, nullptr};

    unsigned int flags = MHD_NO_FLAG;

    if (debug_output_)
    {
        LOG_DEBUG("Enabling debug output");
        flags |= MHD_USE_DEBUG;
    }

    if (ssl_mem_cert_set_ && ssl_mem_key_set_)
    {
        LOG_DEBUG("Enabling SSL");
        flags |= MHD_USE_SSL;
    }
    else if (ssl_mem_cert_set_ || ssl_mem_key_set_)
    {
        LOG_FATAL("Please provide both server::https_mem_key AND server::https_mem_cert");
        return;
    }

    if (use_thread_per_connection_)
    {
        LOG_DEBUG("Will use one thread per connection")
        flags |= MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL;
    }
    else if (use_epoll_if_available_)
    {
#if defined(__linux__)
        LOG_DEBUG("Will use epoll");
        flags |= MHD_USE_EPOLL_INTERNALLY;
#else
        LOG_DEBUG("Will use poll");
        flags |= MHD_USE_POLL_INTERNALLY;
#endif
    }
    else
    {
        LOG_DEBUG("No threading options set, will use poll");
        flags |= MHD_USE_POLL_INTERNALLY;
    }

    daemon_ = MHD_start_daemon(flags,
                               port_,
                               access_policy_callback_shim_, this,
                               access_handler_callback_shim_, this,
                               MHD_OPTION_NOTIFY_COMPLETED, request_completed_callback_shim_, this,
                               MHD_OPTION_EXTERNAL_LOGGER, logger_callback_shim_, nullptr,
                               MHD_OPTION_URI_LOG_CALLBACK, uri_logger_callback_shim_, nullptr,
                               MHD_OPTION_ARRAY, options,
                               MHD_OPTION_END);

    if (!daemon_)
    {
        LOG_FATAL("Luna server failed to start (are you already running something on port " + std::to_string(port_) + "?)"); //TODO set some real error flags perhaps?
        return;
    }



    LOG_INFO("Luna server created on port " + std::to_string(port_));
}

bool server::server_impl::is_running()
{
    return (daemon_ != nullptr);
}

void server::server_impl::stop()
{
    if (daemon_)
    {
        MHD_stop_daemon(daemon_);
        LOG_INFO("Luna server stopped");
        daemon_ = nullptr;
    }
}


server::port server::server_impl::get_port()
{
    return port_;
}

server::server_impl::~server_impl()
{
    stop();
}


server::request_handler_handle server::server_impl::handle_request(request_method method,
                                                                   std::regex &&path,
                                                                   server::endpoint_handler_cb callback)
{
    std::lock_guard<std::mutex> guard{lock_};
    return std::make_pair(method, request_handlers_[method].insert(std::end(request_handlers_[method]), std::make_pair(std::move(path), callback)));
}

server::request_handler_handle server::server_impl::handle_request(request_method method,
                                                                   const std::regex &path,
                                                                   server::endpoint_handler_cb callback)
{
    std::lock_guard<std::mutex> guard{lock_};
    return std::make_pair(method, request_handlers_[method].insert(std::end(request_handlers_[method]), std::make_pair(path, callback)));
}

server::request_handler_handle server::server_impl::serve_files(std::string &&mount_point,
                                                                std::string &&path_to_files)
{
    // TODO this is very very slow. Why is that? Need to profile.
    // TODO also there is no cacheing. We could cache things in memory too!
    std::regex regex{mount_point + "(.*)"};
    return handle_request(request_method::GET, regex, [=](const request &req) -> response
        {
            std::string path = path_to_files + "/" + req.matches[1];

            LOG_DEBUG(std::string{"File requested:  "}+req.matches[1]);
            LOG_DEBUG(std::string{"Serve from    :  "}+path);

            std::string line;
            std::stringstream out;

            // determine MIME type
            magic_t magic_cookie;
            magic_cookie = magic_open(MAGIC_MIME);
            if (magic_cookie == NULL) {
                return {500};
            }
            if (magic_load(magic_cookie, NULL) != 0) {
                magic_close(magic_cookie);
                return {500};
            }

            std::string magic_full{magic_file(magic_cookie, path.c_str())};
            magic_close(magic_cookie);

            //Read and send the file. Notice that we load the whole damned thing up in memory.
            // TODO find a better way to stream these things instead!
            std::ifstream f{path};
            if (f.is_open())
            {
                while ( std::getline (f,line) )
                {
                    out << line << '\n';
                }
                f.close();
                return {magic_full, out.str()};
            }

            return {404};
        });
}

server::request_handler_handle server::server_impl::serve_files(const std::string &mount_point,
                                                                const std::string &path_to_files)
{
    return handle_request(request_method::GET, std::regex{"/test.txt"}, [](const request &req) -> response
        {
            return {404};
        });
}


void server::server_impl::remove_request_handler(request_handler_handle item)
{
    //TODO this is expensive. Find a better way to store this stuff.
    //TODO validate we are receiving a valid iterator!!
    std::lock_guard<std::mutex> guard{lock_};
    request_handlers_[item.first].erase(item.second);
}


int parse_kv_(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    auto kv = static_cast<query_params *>(cls);
    kv->operator[](key) = value ? value : "";
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
    request_method method = method_str_to_enum_(method_char);
    std::string method_str{method_char};

    if (!*con_cls)
    {
        connection_info_struct *con_info = new(std::nothrow) connection_info_struct(method, connection, 65535, iterate_postdata_shim_);
        if (!con_info) return MHD_NO; //TODO what does this mean?

        *con_cls = con_info;

        return MHD_YES;
    }

    //parse the query params:
    std::map<std::string, std::string> header;

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
        if( MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size) == MHD_NO)
        {
            //MHD couldn't parse it, maybe we can.
            con_info->body.append(upload_data, *upload_data_size);
        }

        *upload_data_size = 0; //flags that we processed everything. This is a funny place to put it.
        return MHD_YES;
    }
    else if (!con_info->post_params.empty())//we're done getting postdata, and we have some query params to handle, do something with it
    {
        //if we have post_params, then MHD has ignored the query params. So just overwrite it.
        std::swap(query_params, con_info->post_params);
    }

    std::string url_str{url};

    auto start = std::chrono::system_clock::now();

    LOG_DEBUG(std::string{"Received request for "} + method_str + " " + url_str);

    //iterate through the handlers. Could stand being parallelized, I suppose?
    std::unique_lock<std::mutex> ulock{lock_};
    for (const auto &handler_pair : request_handlers_[method])
    {
        std::smatch pieces_match;
        auto path_regex = std::get<std::regex>(handler_pair);
        auto callback = std::get<endpoint_handler_cb>(handler_pair);

        if (std::regex_match(url_str, pieces_match, path_regex))
        {
            ulock.unlock(); // found a match, can unlock as iterator will not continue

            std::vector<std::string> matches;
            LOG_DEBUG(std::string{"    match: "} + url);
            for (size_t i = 0; i < pieces_match.size(); ++i)
            {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                LOG_DEBUG(std::string{"      submatch "} + std::to_string(i) + ": " + piece);
                matches.emplace_back(sub_match.str());
            }

            response response;
            try
            {
                response = callback({matches, query_params, header, con_info->body});
            }
                //TODO there is surely a more robust way to do this;
            catch (const std::exception &e)
            {
                LOG_ERROR(std::string{"Request handler for \"" + url_str + "\" threw an exception: "} + e.what());
                response = {500, "text/plain", "Internal error"};
                //TODO render the stack trace, etc.
            }
            catch (...)
            {
                LOG_ERROR("Unknown internal error");
                //TODO use the same error message as above, and just log things differently and test for that.
                response = {500, "text/plain", "Unknown internal error"};
                //TODO render the stack trace, etc.
            }

            if (response.status_code == 0) //no status code was provided, assume success
            {
                response.status_code = default_success_code_(method);
            }

            if (response.content_type.empty()) //no content type assigned, use the default
            {
                response.content_type = default_mime_type;
            }

            if (is_error_(response.status_code))
            {
                return render_error_(start, response, connection, url_str, method_str);
            }

            //else render success
            return render_response_(start, response, connection, url_str, method_str);
        }
    }

    /* unsupported HTTP method */
    return render_error_(start, {404}, connection, url, method_str);
}

//TODO this should be a static non-class function, I think.
int server::server_impl::render_response_(const std::chrono::system_clock::time_point &start,
                                          response &response,
                                          MHD_Connection *connection,
                                          const std::string &url,
                                          const std::string &method,
                                          request_headers headers) const
{
    auto mhd_response = MHD_create_response_from_buffer(response.content.length(),
                                                        (void *) response.content.c_str(),
                                                        MHD_RESPMEM_MUST_COPY);

    for(const auto&header : response.headers)
    {
        MHD_add_response_header(mhd_response, header.first.c_str(), header.second.c_str());
    }

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE, response.content_type.c_str());
    auto ret = MHD_queue_response(connection, response.status_code, mhd_response);

    auto end = std::chrono::system_clock::now();

    // log it
    auto client_address = addr_to_str_(MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr);
    auto end_c = std::chrono::system_clock::to_time_t(end);
    std::stringstream sstr;
    auto tm = luna::gmtime(end_c);
    sstr << "[" << luna::put_time(&tm, "%c") << "] " << client_address << " " << method << " " << url << " " << response.status_code << " (" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms)";
    LOG_INFO(sstr.str());

    MHD_destroy_response(mhd_response);
    return ret;
}

int server::server_impl::render_error_(const std::chrono::system_clock::time_point &start, response &response, MHD_Connection *connection, const std::string &url, const std::string &method) const
{
    /* unsupported HTTP method */
    error_handler_callback_(response, method_str_to_enum_(method), url); //hook for modifying response

    return render_response_(start, response, connection, url, method);
}

int server::server_impl::render_error_(const std::chrono::system_clock::time_point &start, response &&response, MHD_Connection *connection, const std::string &url, const std::string &method) const
{
    /* unsupported HTTP method */
    error_handler_callback_(response, method_str_to_enum_(method), url); //hook for modifying response

    return render_response_(start, response, connection, url, method);
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

void * server::server_impl::uri_logger_callback_shim_(void *cls, const char *uri, struct MHD_Connection *con)
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

//    std::cout << "***" << key << ":" << (data ? data : "[null]") << std::endl;

    if(key) //TODO this is a hack, I don't even know if this is a reliable way to detect query params
    {
        auto con_info = static_cast<connection_info_struct *>(cls);
        parse_kv_(&con_info->post_params, kind, key, data);
        return MHD_YES;
    }
//    else
//    {
//        std::cout << "OHNO" << std::endl;
//    }

    return MHD_YES;
}

void server::server_impl::logger_callback_shim_(void *cls, const char *fm, va_list ap)
{
    //not at all happy with this.
    char message[4096];
    std::vsnprintf(message, sizeof(message), fm, ap);
    LOG_DEBUG(message);
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



///// options setting

void server::server_impl::set_option(server::debug_output value)
{
    debug_output_ = static_cast<bool>(value);
}

void server::server_impl::set_option(server::use_thread_per_connection value)
{
    use_thread_per_connection_ = static_cast<bool>(value);
    if(use_epoll_if_available_)
    {
        LOG_ERROR("Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_epoll_if_available");
        use_epoll_if_available_ = false; //not compatible!
    }
}

void server::server_impl::set_option(use_epoll_if_available value)
{
    use_epoll_if_available_ = static_cast<bool>(value);
    if(use_thread_per_connection_)
    {
        LOG_ERROR("Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_thread_per_connection");
        use_thread_per_connection_ = false; //not compatible!
    }
}

void server::server_impl::set_option(const server::mime_type &mime_type)
{
    default_mime_type = mime_type;
}

void server::server_impl::set_option(server::error_handler_cb handler)
{
    error_handler_callback_ = handler;
}

void server::server_impl::set_option(server::port port)
{
    port_ = port;
}

void server::server_impl::set_option(server::accept_policy_cb value)
{
    accept_policy_callback_ = value;
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
    options_.push_back({MHD_OPTION_SOCK_ADDR, 0, const_cast<sockaddr *>(value)});
}

//void server::server_impl::set_option(server::uri_log_callback value)
//{
//    options_.push_back({MHD_OPTION_URI_LOG_CALLBACK, value, NULL});
//}

void server::server_impl::set_option(const server::https_mem_key &value)
{
    // we must make a durable copy of these strings before tossing around char pointers to their internals
    https_mem_key_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_KEY, 0, const_cast<char *>(https_mem_key_[https_mem_key_.size()-1].c_str())});
    ssl_mem_key_set_ = true;
}

void server::server_impl::set_option(const server::https_mem_cert &value)
{
    https_mem_cert_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_CERT, 0, const_cast<char *>(https_mem_cert_[https_mem_cert_.size()-1].c_str())});
    ssl_mem_cert_set_ = true;
}

//void server::server_impl::set_option(server::https_cred_type value)
//{
//    //TODO
//}

void server::server_impl::set_option(const server::https_priorities &value)
{
    https_priorities_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_PRIORITIES, 0, const_cast<char *>(https_priorities_[https_priorities_.size()-1].c_str())});
}

void server::server_impl::set_option(server::listen_socket value)
{
    options_.push_back({MHD_OPTION_LISTEN_SOCKET, value, NULL});
}

void server::server_impl::set_option(server::thread_pool_size value)
{
    options_.push_back({MHD_OPTION_THREAD_POOL_SIZE, value, NULL});
}

void server::server_impl::set_option(server::unescaper_cb value)
{
    unescaper_callback_ = value;
    options_.push_back({MHD_OPTION_UNESCAPE_CALLBACK, (intptr_t) &unescaper_callback_shim_, this});
}

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
    https_mem_trust_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_TRUST, 0, const_cast<char *>(https_mem_trust_[https_mem_trust_.size()-1].c_str())});
}

void server::server_impl::set_option(server::connection_memory_increment value)
{
    options_.push_back({MHD_OPTION_CONNECTION_MEMORY_INCREMENT, static_cast<intptr_t>(value), NULL});
}

//void server::server_impl::set_option(server::https_cert_callback value)
//{
//    //TODO
//}

//void server::server_impl::set_option(server::tcp_fastopen_queue_size value)
//{
//    options_.push_back({MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE, value, NULL});
//}

void server::server_impl::set_option(const server::https_mem_dhparams &value)
{
    https_mem_dhparams_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_DHPARAMS, 0, const_cast<char *>(https_mem_dhparams_[https_mem_dhparams_.size() - 1].c_str())});
}

//void server::server_impl::set_option(server::listening_address_reuse value)
//{
//    options_.push_back({MHD_OPTION_LISTENING_ADDRESS_REUSE, value, NULL});
//}

void server::server_impl::set_option(const server::https_key_password &value)
{
    https_key_password_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_KEY_PASSWORD, 0, const_cast<char *>(https_key_password_[https_key_password_.size() - 1].c_str())});
}

//void server::server_impl::set_option(server::notify_connection value)
//{
//    //TODO
//}


} //namespace luna
