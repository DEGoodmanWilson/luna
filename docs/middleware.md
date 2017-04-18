---
layout: default
title: Implementing middleware
---

# Implementing middleware

Luna supports the notion of middleware. Middleware is code that can be attached to hooks inside of Luna, and that get executed whenever those hooks are hit. At the moment, the following hookpoints are supported:

* `before_request_handler` This hook is executed once a route has been selected, but before a request handler is called, and that can modify the HTTP request as received by Luna

* `after_request_handler` This hook is executed after a request handler has been found and successfully called (_i.e._ not after a 500 result from a crashing request handler, or a 404, but it *is* called if the request handler successfully returns an error), and that can modify the HTTP response before Luna sends it on to the caller.

* `after_error` This hook is executed after a request handler had returned an error, has crashed, or when a matching request handler is not found, and that can modify the HTTP response before Luna sends it on to the caller.

You can attach as many middlewares as you like to each hookpoint. The middlwares will be called in the order that they are attached.

Middleware layers are passed in as an option during server creation.

## Modifying request objects

There are lots of reasons why you might like to modify the request object before hitting your request handler. Perhaps you want to inspect the query parameters for every call made. Or you want to implement a global authentication policy.

Let's suppose we want to insert an arbitrary header into the request object. We create our server, passing in a lambda to insert the new request header:

    luna::server server{
            luna::server::port{8080},

            //insert a simple middleware to append a header
            luna::middleware::before_request_handler{
                    [](luna::request &request)
                    {
                        request.params["boo"] = "scream";
                    }
            }
    };
    
As you can see, the lambda receives a reference to the request object that it is permitted to modify inline.

## Modifying response objects

You might also like to modify the response objects before they are returned to the calling client. Perhaps you want to insert a header to all responses, I don't know.

Let's suppose we want to insert an arbitrary header into the response object. We create our server, passing in a lambda to insert the new response header:

    luna::server server{
            luna::server::port{8080},

            //insert a simple middleware to append a header
            luna::middleware::after_request_handler{
                    [](luna::request &request)
                    {
                        request.params["boo"] = "scream";
                    }
            }
    };

As you can see, the lambda receives a reference to the response object that it is permitted to modify inline. Easy-peasy.