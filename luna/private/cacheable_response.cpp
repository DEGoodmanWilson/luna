//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#include "cacheable_response.h"
#include <iostream>

namespace luna
{

cacheable_response::cacheable_response(struct MHD_Response *mhd_response, luna::status_code status_code)
        : mhd_response{mhd_response}, status_code{status_code}
{
    // TODO remove me
    std::cout << "response created" << std::endl;
}

cacheable_response::~cacheable_response()
{
    if (mhd_response != nullptr)
    {
        MHD_destroy_response(mhd_response);
        std::cout << "response destroyed" << std::endl;
    }
}

//std::shared_ptr<cacheable_response> generate_response_from_file(const luna::response &response)
//{
//
//};
//
//
//std::shared_ptr<cacheable_response> generate_response_(const luna::response &response)
//{
//    // First we need to decide if this response object represents a file on disk, or a buffer in memory.
//    if (!response.file.empty())
//    {
//        return generate_response_from_file(response);
//    }
//
//
//    // That was easy!
//    // TODO is there some way we can pull this out of a cache somewhere?
//    auto my_response = std::make_shared<cacheable_response>();
//
//    my_response->mhd_response = MHD_create_response_from_buffer(response.content.length(),
//                                                        (void *) response.content.c_str(),
//                                                        MHD_RESPMEM_MUST_COPY);
//
//    std::string content_type{response.content_type};
//    if (content_type.empty()) //no content type assigned, use the default
//    {
//        content_type = default_mime_type;
//    }
//
//    // Add headers to response object
//    // TODO we need to DRY this code up
//    for (const auto &header : response.headers)
//    {
//        MHD_add_response_header(my_response->mhd_response, header.first.c_str(), header.second.c_str());
//    }
//    MHD_add_response_header(my_response->mhd_response, MHD_HTTP_HEADER_CONTENT_TYPE, content_type.c_str());
//    MHD_add_response_header(my_response->mhd_response, MHD_HTTP_HEADER_SERVER, server_identifier_.c_str());
//
//    // TODO can we cache this response?
//    return my_response;
//};


} //namespace luna








/*************

int escaped_stat_(const std::string &file, struct stat *st)
{
    auto result = stat(file.c_str(), st);

    if ((result != 0) && (errno == ENOENT))
    {
        //try it again, escaped
        std::string escaped_path{file};

        result = stat(escaped_path.c_str(), st);
    }

    return result;
}


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


void server::server_impl::load_global_headers_(response &response)
{
    for (const auto &header : global_headers_)
    {
        // TODO this would be a great place for c++17 ::merge function
        if (response.headers.count(header.first) == 0)
        {
            LOG_DEBUG("Overriding global header '" + header.first + "' with value '" + header.second + "'");
            response.headers[header.first] = header.second;
        }
    }
}

//TODO this should be a static non-class function, I think.
bool server::server_impl::render_response_(request &request,
                                           response &response,
                                           MHD_Connection *connection)
{
    struct MHD_Response *mhd_response{nullptr};

    load_global_headers_(response);

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
            if (cache_hit)
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

            // TODO replace with new c++17 std::filesystem implementation. Later.
            // first we see if this is a folder or a file. If it is a folder, we look for some index.* files to use instead.

            struct stat st;
            auto stat_ret = escaped_stat_(response.file, &st);

            if(S_ISDIR(st.st_mode))
            {
                LOG_DEBUG("Found folder " + response.file);
                if(response.file[response.file.size()-1] != '/')
                {
                    response.file += "/";
                }
                for(const auto name : index_filenames)
                {
                    std::string induced_filename{response.file + name};
                    LOG_DEBUG("Looking for          : " + induced_filename);
                    stat_ret = escaped_stat_(induced_filename, &st);
                    if(stat_ret == 0)
                    {
                        response.file = induced_filename;
                        break;
                    }
                }
            } else {
                LOG_DEBUG("Not a folder " + response.file);
            }

            if(stat_ret != 0)
            {
                return render_error_(request, {404}, connection);
            }


            /// determine mime type!


            LOG_DEBUG("Actually serving file: "+response.file);
            // TODO sometimes this crashes because of threads colliding. What if we cached these responses?
            auto file = fopen(response.file.c_str(), "r");
            // because we already checked, this is guaranteed to work
            {
                auto fd = fileno(file);
                struct stat stat_buf;
                auto rc = fstat(fd, &stat_buf);
                auto fsize = stat_buf.st_size;

                mhd_response = MHD_create_response_from_fd(fsize, fd);

                if (cache_write_) //only write to the cache if we didn't hit it the first time.
                {
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5
                    #pragma message ( "No support for C++14 lambda captures" )
                    auto writer = cache_write_;
                    auto file = response.file;
                    cache_threads_.emplace_back(std::thread{[writer, file] ()
#else
                    cache_threads_.emplace_back(std::thread{[writer = cache_write_, file = response.file]()
#endif
                                                            {
                                                                std::unique_lock<SHARED_MUTEX> lock{
                                                                        server_impl::cache_mutex_};
                                                                std::ifstream ifs(file);
                                                                writer(file,
                                                                       std::make_shared<std::string>(std::istreambuf_iterator<char>(
                                                                               ifs), std::istreambuf_iterator<char>()));
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

    if (response.content_type.empty()) //no content type assigned, use the default
    {
        if (response.file.empty())
        {
            //serving dynamic content, use the default type
            response.content_type = default_mime_type;
        }
        else
        {
            response.content_type = get_mime_type_(response.file);
        }
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
    access_log(request, response);

    MHD_destroy_response(mhd_response);
    return ret;
}

bool server::server_impl::render_error_(request & request, response & response, MHD_Connection * connection)
{
    // unsupported HTTP method
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
    // unsupported HTTP method
    error_handler_callback_(request, response); //hook for modifying response

    return render_response_(request, response, connection);
}

**********/