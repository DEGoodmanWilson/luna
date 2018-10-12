---
layout: default
title: Serving static assets
---

# Serving static assets

In addition to all the lovely dynamic content you want to serve up, you might well also have some static file that require serving as well—stylesheets, JavaScript, web fonts, images, and so on. Luna, naturally, can help you.

## Using the helper function

Luna provides a helper function to make serving static assets simple. `router::serve_files()` is designed to operate much like the endpoint request handlers: You provide a path on the local filesystem where your assets are located, and an endpoint to mount, and the assets will appear at that endpoint. Here's an example, supposing that you have files located in a folder called `/var/www/public`:

```
/var/www/public/
  - css/
  +-- style.css
  - img/
  +-- hello.jpg
```

And, supposing you wanted to serve these files from `/static`, so that you could access them as `http://example.com/static/css/style.css` and `http://example.com/static/img/hello.jpg`, then you could call `serve_files()` as such:

```cpp
#include <luna/luna.h>

using namespace luna;

int main(void)
{
    luna::server server;

    auto assets = server.create_router("/static"); // set the base endpoint for all files to "/static"
    
    // first param is the endpoint to serve from (relative to "/static")
    // second param is path in the filesystem to mount 
    assets->serve_files("/", "/var/www/public");

    server.start(8443);
}
```

As a benefit, Luna will attempt to determine the MIME type for the served file automatically, but you can always override it by specifying the MIME type in the `luna::response` object.

## Using the response object to load a file

As you've seen in other sections, request handlers return a `luna::response` object that can contain the response body in memory. `luna::response` objects also support attaching a special method for loading the response body from a file. When serving large static assets, this method will ensure that the file is loaded into memory a chunk at a time in a memory-efficient way. Here's a contrived example to demonstrate how you can leverage this feature of `luna::response`. In general, however, you should prefer the mechanism outlined above to this one.

```cpp
#include <luna/luna.h>

using namespace luna;

int main(void)
{
    server server;

    auto assets = server.create_router();

    // Handle a request to load `ginormous.jpg`, which is too big to fit into memory
    assets->handle_request(request_method::GET,
                          "/ginormous.jpg",
                          [](auto req) -> response
                          {
                              // Construct a response object from a path and filename
                              response resp = response::from_file("/var/www/public/img/ginormous.jpg");
                              
                              // We'll need to supply the MIME type on our own, however!
                              resp.content_type = "image/jpg";
                              
                              return resp;
                          });
                          
    server.start(8443);
}
```

----

### < [Prev—Defining endpoints with regexs](regexes.md) | [Next—TLS/HTTPS](https.md) >
