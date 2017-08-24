//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#pragma once

#include <luna/luna.h>
#include "luna/private/cacheable_response.h"
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <thread>

// NOTE: Apple prior to macOS 12 doesn't support shared mutexes :(
// This is a ridiculous hack.
#if defined (__APPLE__)
#include <Availability.h>
#if __apple_build_version__ < 8020000
#pragma message ( "No support for std::shared_lock!" )
#define NO_SHARED_LOCK
#endif
#endif

#if defined(NO_SHARED_LOCK)
#define SHARED_LOCK std::unique_lock
#define SHARED_MUTEX std::mutex
#else
#define SHARED_LOCK std::shared_lock
#define SHARED_MUTEX std::shared_timed_mutex
#endif

namespace luna
{

class response_generator
{
public:
    response_generator();
    ~response_generator();

    std::shared_ptr<cacheable_response> generate_response(const luna::request &request, luna::response &response);

    // option setters
    void set_option(server::error_handler_cb handler);
    void set_option(const server::server_identifier &value);
    void set_option(const server::append_to_server_identifier &value);
    void set_option(const server::mime_type &mime_type);
    void set_option(middleware::after_error value);
    void set_option(server::enable_internal_file_caching &value);
    //static asset caching
    void set_option(std::pair<cache::read, cache::write> value);

    // more options
    void add_global_header(std::string &&header, std::string &&value);
    void add_global_header(const std::string &header, std::string &&value);
    void add_global_header(std::string &&header, const std::string &value);
    void add_global_header(const std::string &header, const std::string &value);

    // error renderers
    server::error_handler_handle handle_error(status_code code, server::error_handler_cb callback);
    void remove_error_handler(server::error_handler_handle item);

//    server::error_handler_handle handle_404(server::error_handler_cb callback);
//    server::error_handler_handle handle_error(status_code code, server::error_handler_cb callback);
//    void remove_error_handler(error_handler_handle item);


private:
    std::shared_ptr<cacheable_response> from_file_(const luna::request &request, luna::response &response);
    void finish_rendering_error_response_(const request &request, response &response) const;

//    std::string server_identifier_;
//    std::unordered_map<std::string, std::shared_ptr<cacheable_response> > fd_cache_;

    std::unordered_map<status_code, cacheable_response> error_respone_cache_;

    std::string default_mime_type_;
    std::string server_identifier_;
    luna::headers global_headers_;

    // static asset caching
    // for the file cache; many threads can read, but we need to restrict writing to one thread.
    // TODO Making this static achieves the desired result of being able to access the mutex even after an instance of
    //  the class is destroyed, but it will be a bottleneck if you have multiple servers with their own independent
    //  caches. We can improve this later.
    //  We can improve this by adding a new cache handler where concurrency is handled in the callbacks themselves.
    //  Maybe.
    static SHARED_MUTEX cache_mutex_;
    std::vector<std::thread> cache_threads_;

    cache::read cache_read_;
    cache::write cache_write_;

    static std::mutex fd_mutex_;

    // fd cache
    bool use_fd_cache_;
    static SHARED_MUTEX fd_cache_mutex_;
    std::unordered_map<std::string, std::shared_ptr<cacheable_response> > fd_cache_;

    // error handling
    server::error_handler_cb error_handler_callback_;
    std::map<status_code, server::error_handler_cb> error_handlers_;
    middleware::after_error middleware_after_error_;
};



} //namespace luna