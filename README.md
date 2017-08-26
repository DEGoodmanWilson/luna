[![Build Status](https://travis-ci.org/DEGoodmanWilson/luna.svg?branch=master)](https://travis-ci.org/DEGoodmanWilson/luna)
[![Coverage Status](https://codecov.io/gh/DEGoodmanWilson/luna/branch/master/graph/badge.svg)](https://codecov.io/gh/DEGoodmanWilson/luna)
[![badge](https://img.shields.io/badge/conan.io-luna%2F3.3.0-green.svg?logo=data:image/png;base64%2CiVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAMAAAAolt3jAAAA1VBMVEUAAABhlctjlstkl8tlmMtlmMxlmcxmmcxnmsxpnMxpnM1qnc1sn85voM91oM11oc1xotB2oc56pNF6pNJ2ptJ8ptJ8ptN9ptN8p9N5qNJ9p9N9p9R8qtOBqdSAqtOAqtR%2BrNSCrNJ/rdWDrNWCsNWCsNaJs9eLs9iRvNuVvdyVv9yXwd2Zwt6axN6dxt%2Bfx%2BChyeGiyuGjyuCjyuGly%2BGlzOKmzOGozuKoz%2BKqz%2BOq0OOv1OWw1OWw1eWx1eWy1uay1%2Baz1%2Baz1%2Bez2Oe02Oe12ee22ujUGwH3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfgBQkREyOxFIh/AAAAiklEQVQI12NgAAMbOwY4sLZ2NtQ1coVKWNvoc/Eq8XDr2wB5Ig62ekza9vaOqpK2TpoMzOxaFtwqZua2Bm4makIM7OzMAjoaCqYuxooSUqJALjs7o4yVpbowvzSUy87KqSwmxQfnsrPISyFzWeWAXCkpMaBVIC4bmCsOdgiUKwh3JojLgAQ4ZCE0AMm2D29tZwe6AAAAAElFTkSuQmCC)](http://www.conan.io/source/luna/3.1.1/DEGoodmanWilson/stable)

# Luna

[Full Documentation](https://don.goodman-wilson.com/luna)

## A web framework in modern C++

A web framework in modern C++, Luna is designed to be easy to use and integrate with any C++ project that needs to serve HTTP endpoints. Luna’s core philosophy is that it should be _easy to use correctly_ and _difficult to use incorrectly_. Of course, it should be robust as well.

You are writing in C++ (because C++ is _awesome_), and your app needs to provide a lightweight HTTP server to communicate with other web services. `libmicrohttpd` is super-awesome, except for that idiomatically C API. Luna is an idiomatically C++ wrapper for `libmicrohttpd` that leans on modern C++ programming concepts.

HTTP server creation, start, shut down, and deletion are all handled behind the scenes with the magic of [RAII](https://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization). Starting a server is automatic with instantiating a `server` object, and allowing your `server` object to fall out of scope is all that is needed to cleanly shut it down. There is nothing else for you to keep track of, or allocate.

Adding endpoints to your server is likewise meant to be simple. Nominate an endpoint with a string or regex and an HTTP verb, and pass in a lambda or other `std::functional`-compatible object (function pointers, bound class member functions), and return a string containing the desired response body. Of course, you can set headers and mime types, too.

## Example code

But don't take my word for it. Here is some code for serving a simple JSON snippet from a single endpoint.

```
#include <string>
#include <luna/luna.h>

using namespace luna;

int main(void)
{
    //start a server delivering JSON by default on the default port 8080
    server server{
        server::mime_type{"application/json"}, //the default is "text/html; charset=UTF-8"
    };

    // Handle GET requests to "localhost:8080/endpoint"
    // Respond with a tiny bit of fun JSON
    server.handle_request(request_method::GET, "/endpoint",
                          [](auto request) -> response
    {
        return {"{\"made_it\": true}"};
    });

    server.await(); //run forever, basically, or until the server decides to kill itself.
}
```

## Prerequisites

A C++14 capable compiler (tested against gcc 4.9, clang 3.6), CMake 2.8. [Conan](https://www.conan.io) for installing dependencies.

## License

[MIT](https://github.com/DEGoodmanWilson/luna/blob/master/LICENSE)

