---
layout: default
title: TLS/HTTPS
---

# {{ page.title }}

Really, if you're deploying your site using, _e.g._ [Now.sh](https://zeit.co/now), or behind [CloudFlare](https://www.cloudflare.com), there isn't much call for you to explicitly add support for TLS to your Luna-built API. But, if you _really must_, read on.

Luna provides full TLS support out of the box. You need only feed Luna two things: A
[server certificate](https://en.wikipedia.org/wiki/Transport_Layer_Security#Digital_certificates) and a
private key. Acquiring these assets is a bit beyond the scope of this document, I am very sorry to say, but a
[quick Google search](https://www.google.com/webhp?q=creating+HTTPS+keys) should get you started. (But see
`examples/TLS.cpp` for a fully working example that uses self-signed keys valid for `localhost`—feel free to use those
keys for development purposes).

Anyway, once you _have_ those two things, you need only pass them to Luna, and Luna will take care of the rest.

```
#include <luna/luna.h>

using namespace luna;

const char* key_pem = R"key(
-----BEGIN RSA PRIVATE KEY-----
foobar
-----END RSA PRIVATE KEY-----
)key";

const char* cert_pem = R"key(
-----BEGIN CERTIFICATE-----
bazqux
-----END CERTIFICATE-----
)key";


int main(void)
{
    // Naturally, this is where you pass the necessary TLS assets to Luna
    server server{server::https_mem_key{key_pem}, server::https_mem_cert{cert_pem}};
    
    auto router = server.create_router();
   
    router->handle_request(request_method::GET,
                          "/hello_world",
                          [](auto req) -> response
                          {
                              return {"<h1>Hello, World!</h1>"};
                          });

    server.start();

    // Open at https://localhost:8080/hello_world
}
```

## Planned improvements

Support for [Let's Encrypt](https://letsencrypt.org/) is on the table for implementation, with the aim of making acquiring the necessary
assets—the server certificate and private key—even simpler. Watch this space.

----

### < [Prev—Serving static assets](static_assets.html) | [Next—Configuration reference](configuration.html) >
