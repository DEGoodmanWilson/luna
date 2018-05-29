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

#pragma once

#include <luna/luna.h>
#include "luna/private/cacheable_response.h"
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <chrono>


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

class response_renderer
{
public:
    response_renderer();

    std::shared_ptr<cacheable_response> render(const luna::request &request, luna::response &response);

    // option setters
    void set_option(const server::server_identifier &value);
    void set_option(const server::server_identifier_and_version &value);
    void set_option(const server::append_to_server_identifier &value); //TODO I am not fond of having this here.
    void set_option(server::enable_internal_file_cache value);
    void set_option(server::internal_file_cache_keep_alive value);

private:
    std::shared_ptr<cacheable_response> from_file_(const luna::request &request, luna::response &response);

    std::string server_identifier_;

    // fd cache
    bool use_fd_cache_;
    static std::mutex fd_mutex_;
    static SHARED_MUTEX fd_cache_mutex_;
    std::unordered_map<std::string, std::shared_ptr<cacheable_response> > fd_cache_;
    std::chrono::milliseconds cache_keep_alive_;
};



} //namespace luna