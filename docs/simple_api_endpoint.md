---
layout: default
title: Defining a simple API endpoint
---

# Defining a simple API endpoint

## Serving static HTML to a simple endpoint

(For a comprehensive example you can try out, look at `examples/basic_webapp.cpp`.)

Suppose we want to handle a request to `/hello_world` by responding with a simple HTML snippet:
    
```html
<h1>Hello, World!</h1>
```

Let's begin by writing a simple function to act as a request handler (nevermind what the request _is_ for the moment).
    
```cpp
#include <luna/luna.h>

using namespace luna;
response hello_world(const request &req)
{
    return {"<h1>Hello, World!</h1>"};
}
```

Don't worry about the parameters to the function for now, since we are returning the HTML snippet unconditionally. The important bit is the return statement, which constructs a new `luna::response` object. This object actually has four components to it: An HTTP status code, a MIME type, a set of response headers, and a string that contains the response body itself. For now we only care about the response body. The status code defaults to the appropriate success code for the HTTP method (201 for POST, 200 for everything else). The MIME type defaults to `"text/html"`. The headers are by default an empty set. So we need only provide the HTML itself.

Now, let's create a `luna::router` to route this request handler, and attach the request handler to the router. We want our server to run on port 8443, and our request handler to trigger on a GET to `/hello_world`.

```cpp
int main(void)
{
    server server;
    auto router = server.create_router();
    
    router->handle_request(request_method::GET,
                          "/hello_world",
                          &hello_world);

    server.start(8443); //run forever, basically, or until the server decides to kill itself.
}
```

The first line simply instantiates an `router` object, which contains the logic for deciding how to route an HTTP request. You can have as many of these as you need. The next line assigns our `hello_word` function to handle `GET` requests made to `/hello_world`.
 
Following that, we have to instantiate a `server`, which handles incoming requests, and makes sure that responses go to the right place. We add our router to our server on the next line. The final line just instructs the server to bind to port 8443, and run forever. 

## Using query parameters

Of course, we could have generated this response dynamically, rather than specifying it with a string constant. Indeed, let's modify our example to echo back the query parameters received.

The type `luna::query_params` is simply an alias for a key-value hash stored as an `std::map`. The keys and the values both are just `std:strings`. So, we might rewrite our request handler as such:

```cpp
response hello_world(const request &req)
{
    std::stringstream body;
    body << "<h1>Hello, World!</h1>\n<ul>\n";

    for(auto& kv : req.params)
    {
        body << "<li><b>" << kv.first << "</b> " << kv.second << "</li>\n";
    }

    body << "</ul>";
    return {body.str()};
}
```
    
        
## Validating parameters

By default, Luna does not examine query parameters in any way. You can tell Luna that particular parameters are required or optional, and how to validate them. Any query parameter that fails validation will result in a 400 error.

When setting up your response handlers, you can pass in a vector of `validator` objects to instruct Luna how to validate the parameters.

Suppose your method at `/hello_world` has a required parameter `name`, and that you don't care what is passed in. You can achieve this with:

```cpp
router.handle_request(request_method::GET,
                      "/hello_world",
                      &hello_world,
                      {
                        {"name", parameter::required}
                      });
```

Perhaps you _do_ care what a valid name looks like. Suppose you will only accept names that contain alphanumeric characters, space, periods, commas, and hyphens—the usual sorts of things you might see in someone's name. But you'd like to exclude semi-colons and other things that might leads to SQL injections.
 
To do this, you can pass in the result of the `parameter::validate()` helper method. This handy method takes as a first parameter a function that takes a string (the query parameter to validate), and returns true if the parameter is valid, and false otherwise. Any subsequent parameters are passed on directly to the validation function when the endpoint is hit.

In this case, we'll use the built-in regex validator, as such:

```cpp
router.handle_request(request_method::GET,
                      "/hello_world",
                      &hello_world,
                      {
                        {"name",
                          parameter::required,
                          parameter::validate(parameter::regex,
                                              std::regex{"[a-zA-Z\\.\\-\\,"]+})}
                      });
```
                                  
Luna offers two other built-in validators: One that validates only exact matches called `parameter::match` (useful for verifying, _e.g._ verficiation tokens), and one that validates integer numbers called `parameter::number`. Future version of Luna may provide additional built-in validation functions.

Of course, you can also write your own validation functions. Suppose we wanted to validate that a parameter is no longer than 10 characters. We could do that with a lambda:

```cpp
router.handle_request(request_method::GET,
                      "/hello_world",
                      &hello_world,
                      {
                        {
                          "name",
                          parameter::required,
                          parameter::validate([](const std::string &a, int length) -> bool
                            {
                                return a.length() <= length;
                            },
                          10)
                        }
                      });
```

## Setting the status code

The response object contains the status code representing the success or failure of a request. By default, the status code is set to either 201 (for POST requests) or 200 (for all other requests). This is easily overridden to indicate other kinds of success, or a failure.

```cpp
    return {404, "<h1>Not Found</h1>"};
```

## Setting the response MIME type

Not all HTTP servers respond with HTML. Many respond with, for example JSON or XML. You can specify a default MIME type for all responses from a `router` object:

```cpp
    router.set_mime_type("text/json");
```

Or you can specify MIME types per response in the response constructor, inside of your response handler function:

```cpp
    return {"text/json", "{\"error\":\"not found\""}};
```

## Setting response headers

If you want to return a custom response header, you can include that in the response object as well, inside of your response handler function.

```cpp
    return {"Hello!", {
        {"My Header", "My Value"},
        {"Another Header", "Another Value"}
    }};
```

----

### < [Prev—Getting started](using.html) | [Next—Defining endpoints with regexs](regexes.html) >