---
layout: default
title: Defining a simple API endpoint
---

# Defining a simple API endpoint

## Serving static HTML to a simple endpoint

(The code discussed here is available in `examples/basic_webapp.cpp`.)

Suppose we want to handle a request to `/hello_world` by responding with a simple HTML snippet:
    
    <h1>Hello, World!</h1>

Let's begin by writing a simple function to act as a request handler (nevermind what the request _is_).
    
    using namespace luna;
    response hello_world(const request &req)
    {
        return {"<h1>Hello, World!</h1>"};
    }

Don't worry about the parameters to the function for now, since we are returning the HTML snippet unconditionally. The important bit is the return statement, which constructs a new `luna::response` object. This object actually has three components to it: An HTTP status code, a MIME type, and a string that contains the response body itself. The status code defaults to the appropriate success code for the HTTP method (201 for POST, 200 for everything else). The MIME type defaults to `"text/html"`. So we need only provide the HTML itself.

Now, let's create a `luna::server` to host this request handler, and attach the request handler to the server. We want our server to run on port 8443, and our request handler to trigger on a GET to `/hello_world`.

    int main(void)
    {
        server server{server::port{8443}};

        server.handle_request(request_method::GET,
                              "/hello_world",
                              &hello_world);

        server.await(); //run forever, basically, or until the server decides to kill itself.
    }

The first line simply instantiates an HTTP server object on port 8443. The final line just loops forever. It is the second line that is of interest. This line instructs our server to listen for GET requests on `/hello_world`, and to call our request handler when it hears such a request.

## Using query parameters

(The code discussed here is available in `examples/example2.cpp`.)

Of course, we could have loaded this HTML from a file, rather than specifying it with a string constant. Or we can generate it dynamically based on the HTTP query parameters. Indeed, let's modify our example to echo back the query parameters received.

`luna::query_params` is simply an alias for a key-value hash stored as an `std::map`. The keys and the values both are just `std:strings`. So, we might rewrite our request handler as such:

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
    
        
## Validating parameters

By default, Luna does not examine query parameters in any way. You can tell Luna that particular parameters are required or optional, and how to validate them. Any query parameter that fails validation will result in a 400 error.

When setting up your response handlers, you can pass in a vector of `validator` objects to instruct Luna how to validate the parameters.

Suppose your method at '/hello_world has a required parameter `name`, and that you don't care what is passed in. You can achieve this with:
 
    server.handle_request(request_method::GET,
                          "/hello_world",
                          &hello_world,
                          {
                            {"name", parameter::required}
                          });

Perhaps you _do_ care what a valid name looks like. Suppose you will only accept names that contain alphanumeric characters, space, periods, commas, and hyphens—the usual sorts of things you might see in someone's name. But you'd like to exclude semi-colons and other things that might leads to SQL injections.
 
To do this, you can pass in the result of the `parameter::validate()` helper method. This handy method takes as a first parameter a function that takes a string (the query parameter to validate), and returns true if the parameter is valid, and false otherwise. Any subsequent parameters are passed on directly to the validation function when the endpoint is hit.

In this case, we'll use the built-in regex validator, as such:

    server.handle_request(request_method::GET,
                          "/hello_world",
                          &hello_world,
                          {
                            {"name",
                              parameter::required,
                              parameter::validate(parameter::regex,
                                                  std::regex{"[a-zA-Z\\.\\-\\,"]+})}
                          });
                                  
Luna offers two other built-in validators: One that validates only exact matches called `parameter::match` (useful for verifying, _e.g._ verficiation tokens), and one that validates integer numbers called `parameter::number`. Future version of Luna may provide additional built-in validation functions.

Of course, you can also write your own validation functions. Suppose we wanted to validate that a parameter is no longer than 10 characters. We could do that with a lambda:

    server.handle_request(request_method::GET,
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

## Setting the status code

The response object contains the status code representing the success or failure of a request. By default, the status code is set to either 201 (for POST requests) or 200 (for all other requests). This is easily overriden to indicate other kinds of success, or a failure.

    return {404, "<h1>Not Found</h1>"}

## Setting the response MIME type

Not all HTTP servers respond with HTML. Many respond with, for example JSON or XML. You can specify a default MIME type for all responses when constructing a new server:

    server server{server::port{8334}, server::mime_type{"text/json"}};

Or you can specify MIME types per response in the response contructor:

    return {"text/json", "{\"error\":\"not found\""}};

## Setting response headers

If you want to return a custom response header, you can include that in the response object as well.

    return {"Hello!", {
        {"My Header", "My Value"},
        {"Another Header", "Another Value"}
    }};
