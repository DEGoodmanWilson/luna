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
#include <sys/stat.h>
#include <arpa/inet.h>
#include "luna/private/server_impl.h"
#include "luna/config.h"

#ifdef LUNA_TESTING
#define STATIC
#else
#define STATIC static
#endif

namespace luna
{

//TODO do this better. Make this an ostream with a custom function. It's not like we haven't done that before.

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

STATIC const server::error_handler_cb default_error_handler_callback_ = [](const request &request,
                                                                           response &response)
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

STATIC const server::accept_policy_cb default_accept_policy_callback_ = [](const struct sockaddr *addr,
                                                                           socklen_t len) -> bool
{
    return true;
};

STATIC status_code default_success_code_(request_method method)
{
    if (method == request_method::POST)
    {
        return 201;
    }

    return 200;
}

STATIC bool is_error_(status_code code)
{
    if (code < 300) return false;

    return true;
}

STATIC bool is_redirect_(status_code code)
{
    if ((code >= 300) && code < 400) return true;

    return false;
}

STATIC request_method method_str_to_enum_(const char *method_str)
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
STATIC request_method method_str_to_enum_(const std::string &method_str)
{
    return method_str_to_enum_(method_str.c_str());
}

STATIC std::string addr_to_str_(const struct sockaddr *addr)
{
    if (addr)
    {
        char str[INET_ADDRSTRLEN];
        //TODO how do we know if we have a v6 address here?
        if (inet_ntop(AF_INET, addr, str, INET_ADDRSTRLEN) == NULL)
        {
            return "";
        }
        return str; //makes a copy, since we are returning an std::string
    }
    return "";
}

STATIC MHD_ValueKind method_to_value_kind_enum_(request_method method)
{
    if (method == request_method::GET)
    {
        return MHD_GET_ARGUMENT_KIND;
    }

    return MHD_POSTDATA_KIND;
}
///////////////////////////

SHARED_MUTEX server::server_impl::cache_mutex_;

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
        port_{8080},
        server_identifier_{std::string{LUNA_NAME} + "/" + LUNA_VERSION},
        cache_read_{nullptr},
        cache_write_{nullptr}
{}


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
        LOG_FATAL("Luna server failed to start (are you already running something on port " + std::to_string(port_) +
                  "?)"); //TODO set some real error flags perhaps?
        return;
    }
    running_cv_.notify_all(); //daemon_ has changed value



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
        //Wait for any pending cache operations to finish.
        for(auto & t : cache_threads_)
        {
            if(t.joinable())
            {
                t.join();
            }
        }

        MHD_stop_daemon(daemon_);
        LOG_INFO("Luna server stopped");
        daemon_ = nullptr;
        running_cv_.notify_all(); //daemon_ has changed value
    }
}

void server::server_impl::await()
{
    std::mutex m;
    {
        std::unique_lock<std::mutex> lk(m);
        running_cv_.wait(lk, [this] { return daemon_ == nullptr; });
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
                                                                   server::endpoint_handler_cb callback,
                                                                   parameter::validators &&validators)
{
    std::lock_guard<std::mutex> guard{lock_};
    return std::make_pair(method,
                          request_handlers_[method].insert(std::end(request_handlers_[method]),
                                                           std::make_tuple(std::move(path),
                                                                           callback,
                                                                           std::move(validators))));
}

server::request_handler_handle server::server_impl::handle_request(request_method method,
                                                                   const std::regex &path,
                                                                   server::endpoint_handler_cb callback,
                                                                   parameter::validators &&validators)
{
    std::lock_guard<std::mutex> guard{lock_};
    return std::make_pair(method,
                          request_handlers_[method].insert(std::end(request_handlers_[method]),
                                                           std::make_tuple(path, callback, std::move(validators))));
}

server::request_handler_handle server::server_impl::handle_request(request_method method,
                                                                   std::regex &&path,
                                                                   server::endpoint_handler_cb callback,
                                                                   const parameter::validators &validators)
{
    std::lock_guard<std::mutex> guard{lock_};
    return std::make_pair(method,
                          request_handlers_[method].insert(std::end(request_handlers_[method]),
                                                           std::make_tuple(std::move(path), callback, validators)));
}

server::request_handler_handle server::server_impl::handle_request(request_method method,
                                                                   const std::regex &path,
                                                                   server::endpoint_handler_cb callback,
                                                                   const parameter::validators &validators)
{
    std::lock_guard<std::mutex> guard{lock_};
    return std::make_pair(method,
                          request_handlers_[method].insert(std::end(request_handlers_[method]),
                                                           std::make_tuple(path, callback, validators)));
}

server::request_handler_handle server::server_impl::serve_files(const std::string &mount_point,
                                                                const std::string &path_to_files)
{
    std::regex regex{mount_point + "(.*)"};
    std::string local_path{path_to_files + "/"};
    return handle_request(request_method::GET, regex, [=](const request &req) -> response
    {
        std::string path = local_path + req.matches[1];

        LOG_DEBUG(std::string{"File requested:  "} + req.matches[1]);
        LOG_DEBUG(std::string{"Serve from    :  "} + path);

        return response::from_file(path);
    });
}

server::request_handler_handle server::server_impl::serve_files(std::string &&mount_point,
                                                                std::string &&path_to_files)
{
    std::regex regex{std::move(mount_point) + "(.*)"};
    std::string local_path{std::move(path_to_files) + "/"};
    return handle_request(request_method::GET, regex, [=](const request &req) -> response
    {
        std::string path = local_path + req.matches[1];

        LOG_DEBUG(std::string{"File requested:  "} + req.matches[1]);
        LOG_DEBUG(std::string{"Serve from    :  "} + path);

        return response::from_file(path);
    });
}

void server::server_impl::remove_request_handler(request_handler_handle item)
{
    //TODO this is expensive. Find a better way to store this stuff.
    //TODO validate we are receiving a valid iterator!!
    std::lock_guard<std::mutex> guard{lock_};
    request_handlers_[item.first].erase(item.second);
}

server::error_handler_handle server::server_impl::handle_404(server::error_handler_cb callback)
{
    return handle_error(404, callback);
}

server::error_handler_handle server::server_impl::handle_error(status_code code, server::error_handler_cb callback)
{
    std::lock_guard<std::mutex> guard{lock_};
    error_handlers_[code] = callback;
    return code;
}

void server::server_impl::remove_error_handler(error_handler_handle item)
{
    //TODO this is expensive. Find a better way to store this stuff.
    //TODO validate we are receiving a valid iterator!!
    std::lock_guard<std::mutex> guard{lock_};
    error_handlers_.erase(item);
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
    else if (!con_info->post_params.empty())//we're done getting postdata, and we have some query params to handle, do something with it
    {
        //if we have post_params, then MHD has ignored the query params. So just overwrite it.
        std::swap(query_params, con_info->post_params);
    }

    // construct request object
    auto ip_address = addr_to_str_(MHD_get_connection_info(connection,
                                                           MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr);

    request req{start, start, ip_address, method, url_str, http_version, {}, query_params, header, con_info->body};

    LOG_DEBUG(std::string{"Received request for "} + method_str + " " + url_str);



    //iterate through the handlers. Could stand being parallelized, I suppose?
    std::unique_lock<std::mutex> ulock{lock_};
    for (const auto &handler_tuple : request_handlers_[method])
    {
        std::smatch pieces_match;
        auto path_regex = std::get<std::regex>(handler_tuple);

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

            req.matches = matches;
            response response;

            auto callback = std::get<endpoint_handler_cb>(handler_tuple);
            try
            {
                // Validate the parameters passed in
                // TODO this can probably be optimized
                bool valid_params = true;
                auto validators = std::get<parameter::validators>(handler_tuple);
                for (const auto &validator : validators)
                {
                    bool present = (query_params.count(validator.key) == 0) ? false : true;
                    if (present)
                    {
                        //run the validator
                        if (!validator.validation_func(query_params[validator.key]))
                        {
                            std::string error{"Request handler for \"" + url_str + " is missing required parameter \"" +
                                              validator.key};
                            LOG_ERROR(error);
                            response = {400, "text/plain", error};
                            valid_params = false;
                            break; //stop examining params
                        }
                    }
                    else if (validator.required) //not present, but required
                    {
                        std::string error{"Request handler for \"" + url_str + " is missing required parameter \"" +
                                          validator.key};
                        LOG_ERROR(error);
                        response = {400, "text/plain", error};
                        valid_params = false;
                        break; //stop examining params
                    }

                }

                if (valid_params)
                {
                    //made it this far! try the callback

                    //first, the before middlewares
                    for (const auto &mw : middleware_before_request_handler_.funcs)
                    {
                        mw(req);
                    }

                    response = callback(req);

                    //now, the after middlewares
                    for (const auto &mw : middleware_after_request_handler_.funcs)
                    {
                        mw(response);
                    }
                }
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
                if (response.file.empty())
                {
                    //serving dynamic content, use the default type
                    response.content_type = default_mime_type;
                }
                else
                {
                    // We are serving a static asset, Calculate the MIME type if not specified
                    luna::response error_response{500}; //in case bad things happen
                    magic_t magic_cookie;
                    magic_cookie = magic_open(MAGIC_MIME);
                    if (magic_cookie == NULL)
                    {
                        // These lines should basically never get hit in testing
                        response.status_code = 500;                                                 //LCOV_EXCL_LINE
                        // I am dubious that if we had an issue allocating memory above that the following will work, TBH
                        return render_error_(req, error_response, connection);   //LCOV_EXCL_LINE
                    }
                    if (magic_load(magic_cookie, NULL) != 0)
                    {
                        magic_close(magic_cookie);                                                  //LCOV_EXCL_LINE
                        response.status_code = 500;                                                 //LCOV_EXCL_LINE
                        return render_error_(req, error_response, connection);   //LCOV_EXCL_LINE
                    }

                    std::string magic_full{magic_file(magic_cookie, response.file.c_str())};
                    magic_close(magic_cookie);
                    response.content_type = magic_full;
                }
            }

            if (is_error_(response.status_code))
            {
                //now, the after_error middlewares
                for (const auto &mw : middleware_after_error_.funcs)
                {
                    mw(response);
                }
                return render_error_(req, response, connection);
            }

            //else render success
            return render_response_(req, response, connection);
        }
    }

    /* unsupported HTTP method */
    //now, the after_error middlewares TODO this could be refactored. The logic is starting to become tortured.
    response response{404};
    for (const auto &mw : middleware_after_error_.funcs)
    {
        mw(response);
    }
    return render_error_(req, response, connection);
}

//TODO this should be a static non-class function, I think.
bool server::server_impl::render_response_(request &request,
                                          response &response,
                                          MHD_Connection *connection)
{
    struct MHD_Response *mhd_response{nullptr};

    //TODO allow callbacks in the response object, in which case use `MHD_create_response_from_callback`

    // We want to serve a file, in which case use `MHD_create_response_from_fd`
    // TODO this mhd_response could be cached to speed things up!
    if (!response.file.empty()) //we have a filename, load up that file and ignore the rest
    {
        //first, let's check the cache!
        if (cache_read_)
        {
            SHARED_LOCK<SHARED_MUTEX> lock{server_impl::cache_mutex_};
            auto cache_hit = cache_read_(response.file);
            if(cache_hit)
            {
                response.content = cache_hit->c_str();
                mhd_response = MHD_create_response_from_buffer(response.content.length(),
                                                               (void *) response.content.c_str(),
                                                               MHD_RESPMEM_MUST_COPY);
            }
        }

        if (response.content.empty())
        {
            //cache miss, or missing cache: look for the file on disk

            response.status_code = 200; //default success

            auto file = fopen(response.file.c_str(), "r");
            if (!file)
            {
                return render_error_(request, {404}, connection);
            }
            else
            {
                auto fd = fileno(file);
                struct stat stat_buf;
                auto rc = fstat(fd, &stat_buf);
                auto fsize = stat_buf.st_size;

                mhd_response = MHD_create_response_from_fd(fsize, fd);

                if(cache_write_) //only write to the cache if we didn't hit it the first time.
                {
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5
#pragma message ( "No support for C++14 lambda captures" )
                    auto writer = cache_write_;
                    auto file = response.file;
                    cache_threads_.emplace_back(std::thread{[writer, file] ()
#else
                    cache_threads_.emplace_back(std::thread{[writer = cache_write_, file = response.file] ()
#endif
                    {
                        std::unique_lock<SHARED_MUTEX> lock{server_impl::cache_mutex_};
                        std::ifstream ifs(file);
                        writer(file, std::make_shared<std::string>(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()));
                    }});
                }
            }
        }
    }
    // we're loading this from the buffer in response.content, no problem.
    else
    {
        mhd_response = MHD_create_response_from_buffer(response.content.length(),
                                                       (void *) response.content.c_str(),
                                                       MHD_RESPMEM_MUST_COPY);

    }

    for (const auto &header : response.headers)
    {
        MHD_add_response_header(mhd_response, header.first.c_str(), header.second.c_str());
    }

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE, response.content_type.c_str());

    MHD_add_response_header(mhd_response, MHD_HTTP_HEADER_SERVER, server_identifier_.c_str());

    bool ret = MHD_queue_response(connection, response.status_code, mhd_response);

    request.end = std::chrono::system_clock::now();

    // log it
    auto end_c = std::chrono::system_clock::to_time_t(request.end);
    std::stringstream sstr;
    auto tm = luna::gmtime(end_c);
    access_log(request);

    MHD_destroy_response(mhd_response);
    return ret;
}

bool server::server_impl::render_error_(request & request, response & response, MHD_Connection * connection)
{
    /* unsupported HTTP method */
    error_handler_callback_(request, response); //hook for modifying response

    // get custom error page if exists
    if (error_handlers_.count(response.status_code))
    {
        error_handlers_.at(response.status_code)(request, response); //re-render response
    }

    return render_response_(request, response, connection);
}

bool server::server_impl::render_error_(request & request, response && response, MHD_Connection * connection)
{
    /* unsupported HTTP method */
    error_handler_callback_(request, response); //hook for modifying response

    return render_response_(request, response, connection);
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

//    std::cout << "***" << key << ":" << (data ? data : "[null]") << std::endl;

    if (key) //TODO this is a hack, I don't even know if this is a reliable way to detect query params
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
    if (use_epoll_if_available_)
    {
        LOG_ERROR(
                "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_epoll_if_available");
        use_epoll_if_available_ = false; //not compatible!
    }
}

void server::server_impl::set_option(use_epoll_if_available value)
{
    use_epoll_if_available_ = static_cast<bool>(value);
    if (use_thread_per_connection_)
    {
        LOG_ERROR(
                "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_thread_per_connection");
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
    options_.push_back({MHD_OPTION_HTTPS_MEM_KEY, 0,
                        const_cast<char *>(https_mem_key_[https_mem_key_.size() - 1].c_str())});
    ssl_mem_key_set_ = true;
}

void server::server_impl::set_option(const server::https_mem_cert &value)
{
    https_mem_cert_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_MEM_CERT, 0,
                        const_cast<char *>(https_mem_cert_[https_mem_cert_.size() - 1].c_str())});
    ssl_mem_cert_set_ = true;
}

//void server::server_impl::set_option(server::https_cred_type value)
//{
//    //TODO
//}

void server::server_impl::set_option(const server::https_priorities &value)
{
    https_priorities_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_PRIORITIES, 0,
                        const_cast<char *>(https_priorities_[https_priorities_.size() - 1].c_str())});
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
    options_.push_back({MHD_OPTION_HTTPS_MEM_TRUST, 0,
                        const_cast<char *>(https_mem_trust_[https_mem_trust_.size() - 1].c_str())});
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
    options_.push_back({MHD_OPTION_HTTPS_MEM_DHPARAMS, 0,
                        const_cast<char *>(https_mem_dhparams_[https_mem_dhparams_.size() - 1].c_str())});
}

//void server::server_impl::set_option(server::listening_address_reuse value)
//{
//    options_.push_back({MHD_OPTION_LISTENING_ADDRESS_REUSE, value, NULL});
//}

void server::server_impl::set_option(const server::https_key_password &value)
{
    https_key_password_.emplace_back(value);
    options_.push_back({MHD_OPTION_HTTPS_KEY_PASSWORD, 0,
                        const_cast<char *>(https_key_password_[https_key_password_.size() - 1].c_str())});
}

//void server::server_impl::set_option(server::notify_connection value)
//{
//    //TODO
//}

void server::server_impl::set_option(const server::server_identifier &value)
{
    server_identifier_ = value;
}

void server::server_impl::set_option(const server::append_to_server_identifier &value)
{
    server_identifier_ += " " + value;
}

void server::server_impl::set_option(middleware::before_request_handler value)
{
    middleware_before_request_handler_ = value;
}

void server::server_impl::set_option(middleware::after_request_handler value)
{
    middleware_after_request_handler_ = value;
}

void server::server_impl::set_option(middleware::after_error value)
{
    middleware_after_error_ = value;
}

void server::server_impl::set_option(std::pair<cache::read, cache::write> value)
{
    cache_read_ = std::get<cache::read>(value);
    cache_write_ = std::get<cache::write>(value);
}

} //namespace luna
