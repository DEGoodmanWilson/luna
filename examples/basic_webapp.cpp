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
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

/////////////////
//
// basic_webapp
//
// A demonstration for serving simple webpages with luna.
// This app defines a handful of endpoints to demonstrate luna usage, and servers very simple content on those endpoints
// This app also demonstrates how to set up logging and other configurable features of luna.
//
/////////////////

#include <string>
#include <sstream>
#include <iostream>
#include <luna/luna.h>

using namespace luna;

////
// Luna provides two logs: the access log, and the error log, similar to Apache.
// The access log is a record of all visits to your site
// The error log helps with debugging and error management
// Let's just dump them to the console
void ex_error_log(log_level level, const std::string &message)
{
    std::cout << "[" << to_string(level) << "] " << message << std::endl;
}

void ex_access_log(const luna::request &request, const luna::response &response)
{
    std::cout << request.ip_address << ": " << luna::to_string(request.method) << " [" << response.status_code << "] "
              << request.path << " " << request.http_version << " " << (request.headers.count("user-agent") ? request.headers.at("user-agent") : "[no user-agent]") << " { "
              << std::chrono::duration_cast<std::chrono::microseconds>(request.end - request.start).count() << "us } " << std::endl;
}

////
// Some methods for handling HTTP requests. We'll come back to these later.
response hello_world_func(const request &req)
{
    return {"<h1>Hello, World!</h1><p>from a function pointer</p>"};
}

struct hello_world_class
{
    response method(const request &req)
    {
        return {"<h1>Hello, World</h1><p>from a class method</p>"};
    }
};





////
// And now the real show

int main(void)
{
    // Set up the logging
    set_error_logger(ex_error_log);
    set_access_logger(ex_access_log);


    // Create the server
    server server{server::enable_internal_file_cache{true}};

    // Create a router for handling requests. We can add lots of routers if we like

    auto router{server.create_router("/")};

    // Example 1: Let's serve some content on /hello_world using a function pointer.
    router->handle_request(request_method::GET, "/function_pointer", &hello_world_func);

    // Example 2: We can do the same thing with a class method
    hello_world_class instance;
    router->handle_request(request_method::GET,
                          "/class_method",
                          std::bind(&hello_world_class::method, &instance, std::placeholders::_1));

    // Example 3: We can even use lambdas, of course
    router->handle_request(request_method::GET, "/lambda", [](const request &req) -> response
    {
        return {"<h1>Hello, World</h1><p>from a lambda</p>"};
    });

    // Example 4: How to access query params and headers! This works the same regardless of the verb.
    // Let's just use lambdas for this.
    // Also, you can see that a good templating engine will go a long way. Luna does not supply a templating engine.
    // http://localhost:8443/parameters?foo=bar&baz=qux
    router->handle_request(request_method::GET, "/parameters", [](const request &req) -> response
    {
        std::stringstream page;
        page << "<h1>Hello, World</h1>\n";

        // we'll just iterate over all the params, and put them into an HTML table, for fun!
        page << "<h2>query parameters</h2>\n<table>\n<th>key</th><th>value</th>\n";
        for(const auto &item : req.params)
        {
            page << "<tr><td>" << item.first << "</td><td>" << item.second << "</td></tr>\n";
        }
        page << "</table>\n";

        // let's do the same with the request headers
        page << "<h2>request headers</h2>\n<table>\n<th>key</th><th>value</th>\n";
        for(const auto &item : req.headers)
        {
            page << "<tr><td>" << item.first << "</td><td>" << item.second << "</td></tr>\n";
        }
        page << "</table>\n";

        return {page.str()};
    });

    // Example 5: Serving up static assets. Wanna serve up static HTML, CSS, JavaScript, images, what have you?
    // You've come to the right example.
    // The filesystem path is relative to the running binary's location. You might have to adjust, depending on
    // how you built this example.
    // http://localhost:8443/static/luna.jpg
    // Let's out this on a different router, just to show how that works too

    auto file_router{server.create_router("/static")}; //serve everything relative to the path "/static"
    file_router->add_header("static-files", "hell yes"); //add a header to all responses from this router->
    file_router->serve_files("/", "tests/public");


    // Example 6: Serving up additional endpoints from a separate base URL

    auto api_router{server.create_router("/api")};
    api_router->handle_request(luna::request_method::GET, "/user", [](const request &req) -> response {
        return {200, "{\"name\": \"Don\",\n\"ID\": \"123\"}"};
    });

    // Start the server on port 8443
    // Block until the server shuts down, which is never. We could also do other things with this thread as well.
    server.start(8443);

    // If we didn't want to block, we could call start_async, which returns immediately. We could then use this thread for other purposes.
    // server.start_async(8443);
    // ...do other stuff!
    // server.stop();
    // server.await(); // wait for the server to shutdown
}