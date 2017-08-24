//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#include <sys/stat.h>
#include <fstream>
#include <magic.h>
#include "response_generator.h"
#include "luna/private/file_helpers.h"
namespace luna
{

bool is_error_(status_code code)
{
    return (code >= 300); // is error is < 300, and not an error if >= 300
}

status_code default_success_code_(request_method method)
{
    if (method == request_method::POST)
    {
        return 201;
    }

    return 200;
}

// TODO should this strip the headers out?
static const server::error_handler_cb default_error_handler_callback_ = [](const request &request,
                                                                           response &response)
{
    if (response.content.empty())
    {
        response.content_type = "text/html; charset=UTF-8";
        //we'd best render it ourselves.
        // TODO move these into error_handlers maybe?
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

//void add_uniform_headers_(std::shared_ptr<cacheable_response> response)
//{
//
//}

std::string get_mime_type_(const std::string &file)
{
// We are serving a static asset, Calculate the MIME type if not specified

// first, let's examine the file extension, we can learn a lot that way, then we fall back on libmagic
    std::string mime_type;

//extract the file extension
    const auto ext_begin = file.find_last_of(".");
    const auto ext = file.substr(ext_begin + 1);
    const auto iter = mime_types.find(ext);
    if (iter != mime_types.end())
    {
        mime_type = iter->second;
    }
    else // fall back on libmagic
    {
        magic_t magic_cookie;
        magic_cookie = magic_open(MAGIC_MIME);
        if (magic_cookie == NULL)
        {
// These lines should basically never get hit in testing
// I am dubious that if we had an issue allocating memory above that the following will work, TBH
            return "";   //LCOV_EXCL_LINE
        }
        if (magic_load(magic_cookie, NULL) != 0)
        {
            magic_close(magic_cookie);                                                  //LCOV_EXCL_LINE
            return "";   //LCOV_EXCL_LINE
        }

        mime_type = magic_file(magic_cookie, file.c_str());
        magic_close(magic_cookie);
    }

    return mime_type;
}

//////////////////////////////////////////////////////////////////////////////

SHARED_MUTEX response_generator::cache_mutex_;
SHARED_MUTEX response_generator::fd_cache_mutex_;
std::mutex response_generator::fd_mutex_;


response_generator::response_generator() :
        default_mime_type_{"text/html; charset=UTF-8"},
        server_identifier_{std::string{LUNA_NAME} + "/" + LUNA_VERSION},
        cache_read_{nullptr},
        cache_write_{nullptr},
        error_handler_callback_{default_error_handler_callback_},
        use_fd_cache_{false}
{}

response_generator::~response_generator()
{
    //Wait for any pending cache operations to finish.
    for (auto &t : cache_threads_)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}


std::shared_ptr<cacheable_response>
response_generator::generate_response(const request &request, response &response)
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
        finish_rendering_error_response_(request, response);

        // add mime type if needed
        if (response.content_type.empty()) //no content type assigned, use the default
        {
            response.content_type = default_mime_type_;
        }

        // Now, create the MHD_Response object
        // TODO it would be nice if we could cache this!
        response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                            (void *) response.content.c_str(),
                                                                                            MHD_RESPMEM_MUST_COPY),
                                                            response.status_code);
    }


    // Add headers to response object, but only if it needs it
    if(!response_mhd->cached)
    {
        for (const auto &header : response.headers)
        {
            MHD_add_response_header(response_mhd->mhd_response, header.first.c_str(), header.second.c_str());
        }
        for (const auto &header : global_headers_)
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

void response_generator::finish_rendering_error_response_(const request &request, response &response) const
{
    // TODO more drying things up
    // render the error response
    if (is_error_(response.status_code))
    {
        // render the error
        error_handler_callback_(request, response);

        // custom error handlers
        if (error_handlers_.count(response.status_code))
        {
            error_handlers_.at(response.status_code)(request, response); //re-render response
        }

        // and call the after_error middlewares
        for (const auto &mw : middleware_after_error_.funcs)
        {
            // TODO update the interface to accept requests.
            mw(response);
        }
    }
}

std::shared_ptr<cacheable_response>
response_generator::from_file_(const request &request, response &response)
{
    std::shared_ptr<cacheable_response> response_mhd;

    //first, let's check the user's in-memory cache!
    if (cache_read_)
    {
        SHARED_LOCK<SHARED_MUTEX> lock{response_generator::cache_mutex_};
        auto cache_hit = cache_read_(response.file);
        if (cache_hit)
        {
            response.content = cache_hit->c_str();
            response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                                (void *) response.content.c_str(),
                                                                                                MHD_RESPMEM_MUST_COPY),
                                                                response.status_code);
        }
    }

    if (!response_mhd)
    {
        // cache miss, or missing cache: look for the file in our local fd cache
        SHARED_LOCK<SHARED_MUTEX> lock{response_generator::cache_mutex_};

        if(use_fd_cache_ &&fd_cache_.count(response.file))
        {
            fd_cache_[response.file]->cached = true;
            return fd_cache_[response.file];
        }

    }


    if (!response_mhd)
    {
        // cache miss, look for the file on disk

        // TODO check local fd cache

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
            response.status_code = 404;
            finish_rendering_error_response_(request, response);

            response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_buffer(response.content.length(),
                                                                                                (void *) response.content.c_str(),
                                                                                                MHD_RESPMEM_MUST_COPY),
                                                                response.status_code);

        }
        else
        {
            // The file _does_ exist, load it up!

            // Made it this far, we have a file of some kind we need to load from the disk, wooo.

            std::unique_lock<std::mutex> fd_lock{
                    response_generator::fd_mutex_};
            // determine mime type
            if(response.content_type.empty())
            {
                response.content_type = get_mime_type_(filename);
            }


            auto file = fopen(filename.c_str(), "r");

            // because we already checked with stat(), this is guaranteed to work

            auto fd = fileno(file);
            auto fsize = st.st_size;

            response_mhd = std::make_shared<cacheable_response>(MHD_create_response_from_fd(fsize, fd),
                                                                response.status_code);

            if (cache_write_) //only write to the cache if we didn't hit it the first time.
            {
//#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5
//#pragma message ( "No support for C++14 lambda captures" )
                auto writer = cache_write_;
                auto filename = response.file;
#define LAMBDA_ARGS writer, filename
//#else
//#define LAMBDA_ARGS writer = cache_write_, filename = response.file
//#endif
                cache_threads_.emplace_back(std::thread{[LAMBDA_ARGS]()
                                                        {
                                                            std::unique_lock<SHARED_MUTEX> cache_lock{
                                                                    response_generator::cache_mutex_};
                                                            std::unique_lock<std::mutex> fd_lock{
                                                                    response_generator::fd_mutex_};
                                                            std::ifstream ifs{res_filename};
                                                            writer(res_filename,
                                                                   std::make_shared<std::string>(std::istreambuf_iterator<char>(
                                                                           ifs), std::istreambuf_iterator<char>()));
                                                        }});
            }
            else if(use_fd_cache_) {
                // write the response to our own fd cache
                // this should be quite fast, so we'll do it synchronously
                // TODO put a cap on how big the cache can be!
                std::unique_lock<SHARED_MUTEX> cache_lock{response_generator::fd_cache_mutex_};
                fd_cache_[response.file] = response_mhd;
            }
        }
    }
    
    return response_mhd;
};


///// Option setters
void response_generator::set_option(server::error_handler_cb handler)
{
    error_handler_callback_ = handler;
}

void response_generator::set_option(const server::server_identifier &value)
{
    server_identifier_ = value;
}

void response_generator::set_option(const server::append_to_server_identifier &value)
{
    server_identifier_ += " " + value;
}

void response_generator::set_option(const server::mime_type &mime_type)
{
    default_mime_type_ = mime_type;
}

void response_generator::set_option(middleware::after_error value)
{
    middleware_after_error_ = value;
}

void response_generator::set_option(server::enable_internal_file_caching &value)
{
    use_fd_cache_ = static_cast<bool>(value);
}

void response_generator::set_option(std::pair<cache::read, cache::write> value)
{
    cache_read_ = std::get<cache::read>(value);
    cache_write_ = std::get<cache::write>(value);
}


void response_generator::add_global_header(std::string &&header, std::string &&value)
{
    global_headers_.emplace(std::move(header), std::move(value));
}

void response_generator::add_global_header(const std::string &header, std::string &&value)
{
    global_headers_.emplace(header, std::move(value));
}

void response_generator::add_global_header(std::string &&header, const std::string &value)
{
    global_headers_.emplace(std::move(header), value);
}

void response_generator::add_global_header(const std::string &header, const std::string &value)
{
    global_headers_.emplace(header, value);
}

server::error_handler_handle response_generator::handle_error(status_code code, server::error_handler_cb callback)
{
    error_handlers_[code] = callback;
    return code;
}

void response_generator::remove_error_handler(server::error_handler_handle item)
{
    //TODO this is expensive. Find a better way to store this stuff.
    //TODO validate we are receiving a valid iterator!!
    error_handlers_.erase(item);
}


} //namespace luna