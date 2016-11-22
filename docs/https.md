---
layout: default
title: TLS/HTTPS
---

# {{ page.title }}

Luna provides full TLS support out of the box. You need only feed Luna two things: A
[server certificate](https://en.wikipedia.org/wiki/Transport_Layer_Security#Digital_certificates) and a
private key. Acquiring these assets is a bit beyond the scope of this document, I am very sorry to say, but a
[quick Google search](https://www.google.com/webhp?q=creating+HTTPS+keys) should get you started. (But see
`example3.cpp` for a fully working example that uses self-signed keys valid for `localhost`—feel free to use those
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
    luna::server server{luna::server::https_mem_key{key_pem}, luna::server::https_mem_cert{cert_pem}};

    // From here, everything is just as you'd expect
    server.handle_request(luna::request_method::GET,
                          "/hello_world",
                          [](auto req) -> luna::response
                          {
                              return {"<h1>Hello, World!</h1>"};
                          });

    while (server); //run forever, basically, or until the server decides to kill itself.

    // Open at https://localhost:8080/hello_world
}
```

## Planned improvements

Support for [Let's Encrypt](https://letsencrypt.org/) is on the table for implementation, with the aim of making acquiring the necessary
assets—the server certificate and private key—even simpler. Watch this space.