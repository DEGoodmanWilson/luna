---
layout: default
title: Defining endpoints with regexes
---

# Defining endpoints with regexes

Sometimes you want to use a regex to capture a range of endpoints in one go. For example: You have a document server, and you want to serve documents using an endpoint like `/documents/[document id]`, where a document id is an `i` followed by 6 hexidecimal digits. You could set up such an endpoint like this:

```cpp
auto router = server.create_router();

router->handle_request(request_method::GET,
    "^/documents/(i[0-9a-f]{6})", 
    [](auto request) -> response
    {
        // TODO...
    });
```

This endpoint will only be invoked if the requested URL matches the regex provided; and the matches are passed on to you. The first match in the vector will be the entire path, not especially useful in this case. Because we defined a match group in the regex around the document id, the second match will contain the document id itself.

Let us suppose that in our filesystem, we have a flat folder full of documents named `[document id].txt`, where a document id consists of a string of exactly 6 lower-case letters and digits. We could return the text documents as such:

```cpp
auto router = server.create_router();

router->handle_request(request_method::GET,
    "^/documents/([0-9a-f]{6})", 
    [](auto request) -> response
    {
        auto doc_id = request.matches[1];
        // load the file named doc_id+."txt" into a std::string called contents

        return {"text/plain", contents};
    }
```

----

### < [Prev—Simple API endpoints](simple_api_endpoint.html) | [Next—Serving static assets](static_assets.html) >
