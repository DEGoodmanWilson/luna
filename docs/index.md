---
layout: default
title: Luna
---

[![Build Status](https://travis-ci.org/DEGoodmanWilson/luna.svg?branch=master)](https://travis-ci.org/DEGoodmanWilson/luna)
[![Coverage Status](https://codecov.io/gh/DEGoodmanWilson/luna/branch/master/graph/badge.svg)](https://codecov.io/gh/DEGoodmanWilson/luna)
[![badge](https://img.shields.io/badge/conan.io-luna%2F{{ site.version }}-green.svg?logo=data:image/png;base64%2CiVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAMAAAAolt3jAAAA1VBMVEUAAABhlctjlstkl8tlmMtlmMxlmcxmmcxnmsxpnMxpnM1qnc1sn85voM91oM11oc1xotB2oc56pNF6pNJ2ptJ8ptJ8ptN9ptN8p9N5qNJ9p9N9p9R8qtOBqdSAqtOAqtR%2BrNSCrNJ/rdWDrNWCsNWCsNaJs9eLs9iRvNuVvdyVv9yXwd2Zwt6axN6dxt%2Bfx%2BChyeGiyuGjyuCjyuGly%2BGlzOKmzOGozuKoz%2BKqz%2BOq0OOv1OWw1OWw1eWx1eWy1uay1%2Baz1%2Baz1%2Bez2Oe02Oe12ee22ujUGwH3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfgBQkREyOxFIh/AAAAiklEQVQI12NgAAMbOwY4sLZ2NtQ1coVKWNvoc/Eq8XDr2wB5Ig62ekza9vaOqpK2TpoMzOxaFtwqZua2Bm4makIM7OzMAjoaCqYuxooSUqJALjs7o4yVpbowvzSUy87KqSwmxQfnsrPISyFzWeWAXCkpMaBVIC4bmCsOdgiUKwh3JojLgAQ4ZCE0AMm2D29tZwe6AAAAAElFTkSuQmCC)](http://www.conan.io/source/luna/{{ site.version }}/DEGoodmanWilson/stable)

# Luna

A static webserver and API framework in modern C++, Luna is designed to serve low latency, high throughput static web applications backed by an API, with an easy to use interface. Yeah, C++. Come at me. Luna is _fast_, and _easy to program_. You'll never go back to Node.js.

Luna’s core philosophy is that it should be _easy to use correctly_ and _difficult to use incorrectly_.

## Why in god's name should I use _Luna_?

An excellent question. The first and most obvious answer is because C++ is _fast_. Also, compiled, statically-typed languages will catch many classes of errors before they make it into deployment, making your web applications and APIs more robust. Lastly, to be completely honest, C++ is _fun_. Or so I think anyway.

## Use cases

* You have a static website or single-page app that needs a robust, responsive API to back it. C++ is your friend, my friend.

* You are writing a web application, and you want it as close to the bare metal as possible to keep response latency to the absolute minimum. You're using modern C++ for its memory safety, and you'd like a web framework that does the same.

* You are writing a server or daemon in C++ (because C++ is _awesome_), and your services needs to provide a lightweight HTTP server to communicate with other web clients. You'd use `libmicrohttpd`, which is also super-awesome except for that idiomatically C API. Luna is an idiomatically C++ wrapper for `libmicrohttpd` that leans on modern C++ programming concepts.

## Why choose C++?

Yeah, C++ _sounds_ scary, but it isn't. Scroll down and look at that example. Do it. That's not _worse_ than Node.js is it? Indeed, it's better, because unlike Node, C++ wears its semantics on its sleeve: There are no side effects, everything is explicit (no hoisting! scopes are explicit, no trying to understand the contents of `this`!), and most errors will be caught at compile time, so you know you are deploying code free from syntax errors.  

## Why choose Luna over another C++ framework?

You speak C++ already? Why should you choose Luna?

HTTP server creation, start, shut down, and deletion are all handled behind the scenes with the magic of [RAII](https://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization). Starting a server is automatic with instantiating a `server` object, and allowing your `server` object to fall out of scope is all that is needed to cleanly shut it down. There is nothing else for you to keep track of, or allocate.

Adding endpoints to your server is likewise meant to be simple. Nominate an endpoint with a string or regex and an HTTP verb, and pass in a lambda or other `std::functional`-compatible object (function pointers, bound class member functions), and return a string containing the desired response body. This kind of flexibility means Luna works with heavily object-oriented code—or more procedural or even functional programming styles, as you prefer.

## Example code

But don't take my word for it. Here is some code for serving a simple JSON snippet from a single endpoint. You can find and run this example in `examples/intro.cpp`

```
#include <string>
#include <iostream>
#include <luna/luna.h>

using namespace luna;

int main(void)
{
    // create the server object
    server server;

    // set up an endpoint that serves some JSON on /endpoint
    auto api = server.create_router("/");

    api->set_mime_type("application/json"); //the default is "text/html; charset=UTF-8"

    // Handle GET requests to "localhost:8080/endpoint"
    // Respond with a tiny bit of fun JSON
    api->handle_request(request_method::GET, "/endpoint",
                       [](auto request) -> response
                       {
                           return {"{\"made_it\": true}"};
                       });

    //start a server on port 8080;
    std::cout << "curl -v http://localhost:8080/endpoint" << std::endl;
    server.start(8080);
}
```

## Prerequisites

A C++14 capable compiler (tested against gcc 4.9, clang 3.6), CMake 2.8. [Conan](https://www.conan.io) for installing dependencies.

## License

[MIT](https://github.com/DEGoodmanWilson/luna/blob/master/LICENSE)

## Disclaimer

This library has only begun, and is a constant state of flux.
