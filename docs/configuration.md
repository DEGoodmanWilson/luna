---
layout: default
title: Configuration options
---

# Global configuration options

## Logger

The Luna logger is a `functional` type that you provide. To set it, you can pass to `luna::set_logger()` a function pointer, a non-static class method via `std::bind`, an `std::function` object, or a lambda with the following signature

    void (luna::log_level, const std::string &)

For example, to log messages to `stdout`, we could write a lambda:

    luna::set_logger([](luna::log_level level, const std::string &message)
    {
        std::cout << to_string(level) < ": " << message << std::endl;
    });

# Server configuration options

As `luna::server` is the object through which all interactions happen, configuration options are set via the `server` contructor. The most important option may well be the port that your `server` object will listen on:

    using namespace luna;
    server server_1{server::port{8446}};
    auto server_2 = std::make_unique<server>({server::port{8447}});

## Named configuration options and ordering

All options are passed to the `server` constructor using the _named option_ pattern: Each option is set using the option name, and the order that options are passed does not matter. Any options not explicitly set are given sensible defaults.

As an example of the _named option_ pattern, let's configure a server on port 7000 and a default MIME type of `"text/json"`:

    server my_server{server::mime_type{"text/json"}, server::port{8446}};

Because with the _named option_ pattern order doesn't matter, we could have just as easily said

    server my_server{server::port{8446}, server::mime_type{"text/json"}};

## Options that are callbacks

# Configuration options

## Variable options

- `port`: The port to run the HTTPD server on.
    
    Default: Whatever `libmicrohttpd` chooses. TODO this is often 0!

- `mime_type`: The default MIME type to serve up.

    Default: `"text/html"`


<!-- //TODO just not going to try to support these two for now
//TODO MHD_OPTION_HTTPS_CERT_CALLBACK cbshim_
//    using notify_connection_cb = std::function<void(struct MHD_Connection *connection, void **socket_context, enum MHD_ConnectionNotificationCode toe)>; -->


- `connection_memory_limit`:

- `connection_limit`:

- `connection_timeout`:

- `per_ip_connection_limit`:

- `sockaddr_ptr`:

- `https_mem_key`:

- `https_mem_cert`:

<!-- //`https_cred_type`: //TODO probably don't need to define this one. -->

- `https_priorities`:

- `listen_socket`:

- `thread_pool_size`:

<!-- //`digest_auth_random`: //TODO unsure how best to support this one -->

- `nonce_nc_size`:

- `thread_stack_size`:

- `https_mem_trust`:

- `connection_memory_increment`:

- `tcp_fastopen_queue_size`:

- `https_mem_dhparams`:

- `listening_address_reuse`:

- `https_key_password`:

## Callback options

- `accept_policy_cb`:

    Signature: `bool cb(const struct sockaddr *, socklen_t)`


- `endpoint_handler_cb`:

    Signature: `response cb(const endpoint_matches &matches, const query_params &params)`

- `error_handler_cb`:

    Signature: `void cb(response &response, request_method method, const std::string &path)`

- `logger_cb`:

    Signature: `void cb(const std::string& message)`

- `unescaper_cb`:

    Signature: `std::string cb(const std::string& text)`

 

