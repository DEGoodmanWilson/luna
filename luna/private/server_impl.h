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

#include "luna/router.h"
#include "luna/private/safer_times.h"
#include "luna/private/response_renderer.h"
#include "luna/server.h"
#include <microhttpd.h>
#include <cstring>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace luna
{



class server::server_impl
{
public:
    server_impl();

    std::mutex lock_;

    bool debug_output_;

    bool ssl_mem_key_set_;
    bool ssl_mem_cert_set_;

    bool use_thread_per_connection_;

    bool use_epoll_if_available_;

    uint16_t port_;

    // string copies of options
    std::vector<std::string> https_mem_key_;
    std::vector<std::string> https_mem_cert_;
    std::vector<std::string> https_priorities_;
    std::vector<std::string> https_mem_trust_;
    std::vector<std::string> https_mem_dhparams_;
    std::vector<std::string> https_key_password_;

    //options
    std::vector<MHD_OptionItem> options_;

    struct MHD_Daemon *daemon_;

    std::condition_variable running_cv_;

    ///// internal use-only callbacks

    int access_handler_callback_(struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method,
                                 const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size,
                                 void **con_cls);


    ////// external-use callbacks that can be set with options
    accept_policy_cb accept_policy_callback_; //has a default value

    unescaper_cb unescaper_callback_;

    ///// callback shims

    static int access_policy_callback_shim_(void *cls,
                                            const struct sockaddr *addr,
                                            socklen_t addrlen);

    static int access_handler_callback_shim_(void *cls,
                                             struct MHD_Connection *connection,
                                             const char *url,
                                             const char *method,
                                             const char *version,
                                             const char *upload_data,
                                             size_t *upload_data_size,
                                             void **con_cls);

    static void request_completed_callback_shim_(void *cls,
                                                 struct MHD_Connection *connection,
                                                 void **con_cls,
                                                 enum MHD_RequestTerminationCode toe);

    static void *uri_logger_callback_shim_(void *cls, const char *uri, struct MHD_Connection *con);

    static void logger_callback_shim_(void *cls, const char *fm, va_list ap);

    static size_t unescaper_callback_shim_(void *cls, struct MHD_Connection *c, char *s);

    static int iterate_postdata_shim_(void *cls,
                                      enum MHD_ValueKind kind,
                                      const char *key,
                                      const char *filename,
                                      const char *content_type,
                                      const char *transfer_encoding,
                                      const char *data,
                                      uint64_t off,
                                      size_t size);

    //TODO MHD_OPTION_HTTPS_CERT_CALLBACK callback_shim_

    //TODO I don't know what to do with this one yet.
//    static void notify_connection_callback_shim_(void *cls,
//                                                 struct MHD_Connection *connection,
//                                                 void **socket_context,
//                                                 enum MHD_ConnectionNotificationCode toe);


    // request handling and response generation
    std::vector<std::shared_ptr<router>> routers_;
    response_renderer response_renderer_;

    std::string server_name_;
};

} //namespace luna
