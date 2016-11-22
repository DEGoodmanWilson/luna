---
layout: default
title: Serving static assets
---

# {{ page.title }}

In addition to all the lovely dynamic content you want to serve up, you might well also have some static file that
require serving as well—stylesheets, JavaScript, web fonts, images, and so on. Luna, naturally, can help you.

## Using the helper function

Luna provides a helper function to make serving static assets simple. `luna::server::serve_files()` is designed to
operate much like the endpoint request handlers: You provide a path on the local filesystem where your assets are
located, and an endpoint to mount, and the assets will appear at that endpoint. Here's an example, supposing that you
have files located in a folder called `/var/www/public`:

```
/var/www/public/
  - css/
  +-- style.css
  - img/
  +-- hello.jpg
```

And, supposing you wanted to serve these files from `/`, so that you could access them as `http://example.com/css/style.css`
and `http://example.com/img/hello.jpg`, then you could call `serve_files()` as such:

```cpp
#include <luna/luna.h>

using namespace luna;

int main(void)
{
    luna::server server{};
    
    // first param is endpoint to mount the filesystem
    // second param is path in the filesystem to mount 
    server.serve_files("/", "/var/www/public");

    while (server); //run forever, basically, or until the server decides to kill itself.
}
```

As a benefit, Luna will attempt to determine the MIME type for the served file automatically, but you can always
override it by specifying the MIME type in the `luna::response` object.

## Using the response object to load a file

As you've seen in other sections, request handlers return a `luna::response` object that can contain the response
body in memory. `luna::response` objects also support attaching a callback for loading the response body from a file.
When serving large static assets, this method will ensure that the file is loaded into memory a chunk at a time in a
memory-effecient way. Here's a contrived example to demonstrate how you can
leverage this feature of `luna::response`.

```cpp
#include <luna/luna.h>

using namespace luna;

int main(void)
{
    luna::server server{};

    // Handle a request to load `ginormous.jpg`, which is too big to fit into memory
    server.handle_request(luna::request_method::GET,
                          "/ginormous.jpg",
                          [=](auto req) -> luna::response
                          {
                              // Construct a response object from a path and filename
                              luna::response resp = luna::response::from_file("/var/www/public/img/ginormous.jpg");
                              
                              // We'll need to supply the MIME type on our own, however!
                              resp.content_type = "image/jpg";
                              
                              return resp;
                          });

    while (server); //run forever, basically, or until the server decides to kill itself.
}
```