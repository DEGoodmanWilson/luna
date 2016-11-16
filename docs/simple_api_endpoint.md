---
layout: default
title: Defining a simple API endpoint
---

# Defining a simple API endpoint

## Serving static HTML to a simple endpoint

(The code discussed here is available in `examples/example1.cpp`.)

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

        while (server);
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

## Setting the status code

The response object contains the status code representing the success or failure of a request. By default, the status code is set to either 201 (for POST requests) or 200 (for all other requests). This is easily overriden to indicate other kinds of success, or a failure.

    return {404, "<h1>Not Found</h1>"}

## Setting the response MIME type

Not all HTTP servers respond with HTML. Many respond with, for example JSON or XML. You can specify a default MIME type for all responses when constructing a new server:

    server server{server::port{8334}, server::mime_type{"text/json"}};

Or you can specify MIME types per response in the response contructor:

    return {"text/json", "{\"error\":\"not found\""}};

## Setting response headers

At this moment, there is no facility for specifying custom response headers.


# TODO 
- no way to construct a response object with binary data, that's a real shame
- responses are constructed in memory. Maybe we don't want that. Maybe we want to provide a hook in a response object for reading data chunks at a time.