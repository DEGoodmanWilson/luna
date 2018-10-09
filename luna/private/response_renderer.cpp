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

#include <sys/stat.h>
#include <fstream>
#include <mime/mime.h>
#include "response_renderer.h"
#include "luna/private/file_helpers.h"

#include <iostream>

namespace luna
{

status_code default_success_code_(request_method method)
{
    if (method == request_method::POST)
    {
        return 201;
    }

    return 200;
}

std::string get_mime_type_(const std::string &file)
{
    std::string retval;
    try
    {
        retval = mime::content_type(mime::get_extension_from_path(file));
    }
    catch(std::out_of_range &e)
    {
        retval = "text/plain";
    }
    return retval;
}

//////////////////////////////////////////////////////////////////////////////

SHARED_MUTEX response_renderer::fd_cache_mutex_;
std::mutex response_renderer::fd_mutex_;


response_renderer::response_renderer() :
        server_identifier_{std::string{LUNA_NAME} + "/" + LUNA_VERSION},
        use_fd_cache_{false},
        cache_keep_alive_{std::chrono::minutes{30}}
{}


std::shared_ptr<cacheable_response>
response_renderer::render(const request &request, response &response)
{
    std::shared_ptr<cacheable_response> response_mhd;

    // Add default status code, if missing
    if (0 == response.status_code)
    {
        response.status_code = default_success_code_(request.method);
    }


    // First we need to decide if this response object represents a file on disk, or a buffer in memory.
    if (!response.file.empty())
    {
        response_mhd = from_file_(request, response);
    }

    else
    {
        // if we got an error, we need to fill it out with some content, etc.
//        finish_rendering_error_response_(request, response);

        // Now, create the MHD_Response object
        // TODO it would be nice if we could cache this!
        response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                            (void *) response.content.c_str(),
                                                                                            MHD_RESPMEM_MUST_COPY),
                                                            response.status_code);
    }


    // Add headers to response object, but only if it needs it
    if (!response_mhd->cached)
    {
        for (const auto &header : response.headers)
        {
            MHD_add_response_header(response_mhd->mhd_response, header.first.c_str(), header.second.c_str());
        }
        MHD_add_response_header(response_mhd->mhd_response,
                                MHD_HTTP_HEADER_CONTENT_TYPE,
                                response.content_type.c_str());
        MHD_add_response_header(response_mhd->mhd_response, MHD_HTTP_HEADER_SERVER, server_identifier_.c_str());
    }

    // TODO can we cache this response?
    return response_mhd;
}

std::shared_ptr<cacheable_response>
response_renderer::from_file_(const request &request, response &response)
{
    std::shared_ptr<cacheable_response> response_mhd;

    if (!response_mhd) // TODO always false! What was this here for?
    {
        // look for the file in our local fd cache
        if (use_fd_cache_ && fd_cache_.count(response.file))
        {
            SHARED_LOCK<SHARED_MUTEX> lock{response_renderer::fd_cache_mutex_};

            response_mhd = fd_cache_[response.file];
            // has the cache expired?
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - response_mhd->time_cached);
            if (duration.count() <= cache_keep_alive_.count())
            {
                response_mhd->cached = true;
                error_log(log_level::DEBUG, "File cache: HIT");
#ifdef LUNA_TESTING
                auto header = MHD_get_response_header(response_mhd->mhd_response, "X-LUNA-CACHE");
                if (header == nullptr)
                {
                    MHD_add_response_header(response_mhd->mhd_response, "X-LUNA-CACHE", "HIT");
                }
                else if (header[0] == 'M')
                {
                    MHD_del_response_header(response_mhd->mhd_response, "X-LUNA-CACHE", "MISS");
                    MHD_add_response_header(response_mhd->mhd_response, "X-LUNA-CACHE", "HIT");
                }
#endif
                return response_mhd; // we can jump out early.
            }

            // else lets invalidate the cache
            fd_cache_.erase(response.file);
            response_mhd = nullptr; //delete our copy too
        }
    }


    // cache miss, look for the file on disk

    // TODO replace with new c++17 std::filesystem implementation. Later.
    // first we see if this is a folder or a file. If it is a folder, we look for some index.* files to use instead.
    struct stat st;
    auto filename = response.file;

    auto stat_ret = stat(filename.c_str(), &st);


    if (S_ISDIR(st.st_mode))
    {
        if (filename[filename.size() - 1] != '/')
        {
            filename += "/";
        }
        for (const auto name : index_filenames)
        {
            std::string induced_filename{filename + name};
            {
                stat_ret = stat(induced_filename.c_str(), &st);
            }
            if (stat_ret == 0)
            {
                filename = induced_filename;
                break;
            }
        }
    }

    if (stat_ret != 0)
    {
        // The file doesn't exist
        // TODO where do we get this 404 from? We need the router that generated this response! Ugh.
        response.status_code = 404;

        response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                            (void *) response.content.c_str(),
                                                                                            MHD_RESPMEM_MUST_COPY),
                                                            response.status_code);

        return response_mhd; // done!

    }


    // Made it this far, we have a file of some kind we need to load from the disk, wooo.

    std::unique_lock<std::mutex> fd_lock{
            response_renderer::fd_mutex_};
    // determine mime type
    if (response.content_type.empty())
    {
        response.content_type = get_mime_type_(filename);
    }

    response.status_code = default_success_code_(request.method);

    auto file = fopen(filename.c_str(), "r");

    // because we already checked with stat(), this is guaranteed to work

    auto fd = fileno(file);
    auto fsize = st.st_size;

    response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_fd(fsize, fd),
                                                        response.status_code);

    if (use_fd_cache_)
    {
        // write the response to our own fd cache
        // this should be quite fast, so we'll do it synchronously
        // TODO put a cap on how big the cache can be!
        std::unique_lock<SHARED_MUTEX> cache_lock{response_renderer::fd_cache_mutex_};
        response_mhd->time_cached = std::chrono::system_clock::now();
        error_log(log_level::DEBUG, "File cache: MISS");
#ifdef LUNA_TESTING
        MHD_add_response_header(response_mhd->mhd_response, "X-LUNA-CACHE", "MISS");
#endif
        fd_cache_[response.file] = response_mhd;
    }

    return response_mhd;
};


///// Option setters

void response_renderer::set_option(const server::server_identifier &value)
{
    server_identifier_ = value.get();
}

void response_renderer::set_option(const server::server_identifier_and_version &value)
{
    server_identifier_ = value.first + "/" + value.second;
}

void response_renderer::set_option(const server::append_to_server_identifier &value)
{
    server_identifier_ += " " + value.get();
}

void response_renderer::set_option(server::enable_internal_file_cache value)
{
    use_fd_cache_ = value.get();
}

void response_renderer::set_option(server::internal_file_cache_keep_alive value)
{
    cache_keep_alive_ = value;
}

} //namespace luna
