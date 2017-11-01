---
layout: default
title: Configuration options
---

Luna has a lot of options. Most of them are based on the options to the underlying `libmicrohttpd` library that Luna wraps.
Options are pretty straightforward to set when creating a server. Read on to learn how to set options, and to learn more
about the options available to be set.

# Global configuration options

## Logger

The Luna logger is a `functional` type that you provide. To set it, you can pass to `luna::set_logger()` a function pointer, a non-static class method via `std::bind`, an `std::function` object, or a lambda with the following signature

```cpp
void (luna::log_level, const std::string &)
```

For example, to log messages to `stdout`, we could write a lambda:

    luna::set_logger([](luna::log_level level, const std::string &message)
    {
        std::cout << to_string(level) < ": " << message << std::endl;
    });

# Server configuration options

As `luna::server` is the object through which all interactions happen, configuration options are set via the `server` contructor. The most important option may well be the port that your `server` object will listen on:

```cpp
using namespace luna;
server server_1{server::port{8446}};
auto server_2 = std::make_unique<server>({server::port{8447}});
```

## Named configuration options and ordering

All options are passed to the `server` constructor using the _named option_ pattern: Each option is set using the option name, and the order that options are passed does not matter. Any options not explicitly set are given sensible defaults.

As an example of the _named option_ pattern, let's configure a server on port 7000 and a default MIME type of `"text/json"`:

```cpp
server my_server{server::mime_type{"text/json"}, server::port{8446}};
```

Because with the _named option_ pattern order doesn't matter, we could have just as easily said

```cpp
server my_server{server::port{8446}, server::mime_type{"text/json"}};
```

## Options that are callbacks

Some options are for configuring callbacks that you provide. These are easy to set up with C++ lambdas, `std::bind`, or
even plain old function pointers.

For example, `error_handler_cb` is an option for rendering custom error pages on, _e.g._ `404` errors.
 
```cpp
void my_error_handler(response &response, request_method method, const std::string &path)
{
    //we'll render some simple HTML
    response.content_type = "text/html; charset=UTF-8";
    switch (response.status_code)
    {
    case 404:
        response.content = "<h1>OH NOES THERE IS NOTHING HERE</h1>";
        break;
    default:
        response.content = "<h1>Yikes!</h1>";
    }
}

...

server my_server{server::handler{&my_error_handler}};

```

# Configuration options reference

## Common options

- `port`: The port to run the HTTPD server on.
    
    Default: `8080`

- `mime_type`: The default MIME type to serve up.

    Default: `"text/html"`
    
- `debug_output`: Enable libmicrohttpd debugging. Under the covers, Luna is a wrapper around libmicrohttpd, and sometimes
  it is useful to turn this option on to debug why a server won't start. Of course, this option does nothing unless you
  have specified a [logging callback](#logger).
  
    Default: `false`

## HTTPS / TLS options

- `https_mem_key`: A string containing the private key to use for TLS. Must be used in conjunction with `https_mem_cert`

- `https_mem_cert`: A string containing the certificate to use for TLS. Must be used in conjunction with `https_mem_key`

<!-- //`https_cred_type`: //TODO probably don't need to define this one. -->

<!-- - `https_priorities`:

- `https_mem_trust`:

- `https_mem_dhparams`:

- `https_key_password`: -->


## Threading options

- `use_thread_per_connection`: Use an independent thread for each connection. Incompatible with `use_epoll_if_available` for reasons. Generally not recommended, but you can give it a try if you like.

    Default: `false`
    
- `thread_pool_size`: Use more than one thread to serve connections. This is a good option to play with.

    Default: 1
    
- `thread_stack_size`: Things and stuff

    Default: system default
    
- `use_epoll_if_available`: Use `epoll` if available, or `poll` otherwise. Propably a good thing to try as well, even if you're not on Linux.

    Default: `false`

## File cacheing options

- `enable_internal_file_cache`: Cache file descriptors. Keeps files open, so they are faster to serve. This means of course that local changes to the filesystem will generally be ignored.

- `internal_file_cache_keep_alive`: How long to hold a file in the cache before invalidating it. Once this interval has passed, the next request for this file will fetch it fresh of the disk. 30 minutes is the default. Only has meaning of you're using `enable_internal_file_cache{true}`.

## Callback options

- `accept_policy_cb`: You can choose to accept or reject connections on the basis of their address. The default is to accept all incoming connections regardless of origin.

    Signature: `bool cb(const struct sockaddr *, socklen_t)`

- `error_handler_cb`: Render a custom error page.

    Signature: `void cb(response &response, request_method method, const std::string &path)`

<!-- - `logger_cb`: Oh, so you'd like to get some logs?

    Signature: `void cb(const std::string& message)` -->

- `unescaper_cb`: You don't like the default URL unescaping algorithm? Offer up your own!

    Signature: `std::string cb(const std::string& text)`

## Options undocumented at the moment because I haven't gotten around to it yet


<!-- //TODO just not going to try to support these two for now
//TODO MHD_OPTION_HTTPS_CERT_CALLBACK cbshim_
//    using notify_connection_cb = std::function<void(struct MHD_Connection *connection, void **socket_context, enum MHD_ConnectionNotificationCode toe)>; -->


- `connection_memory_limit`:

- `connection_limit`:

- `connection_timeout`:

- `per_ip_connection_limit`:

- `sockaddr_ptr`:


- `listen_socket`:

<!-- //`digest_auth_random`: //TODO unsure how best to support this one -->

- `nonce_nc_size`:

- `connection_memory_increment`:

- `tcp_fastopen_queue_size`:

- `listening_address_reuse`:
 
----

### < [Prev—TLS/HTTPS](https.html) | [Next—Using the project template](template.html) >
