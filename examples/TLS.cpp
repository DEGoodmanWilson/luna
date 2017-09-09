//
//      _
//  ___/__)
// (, /      __   _
//   /   (_(_/ (_(_(_
//  (________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

/////////////////
//
// TLS
//
// A demonstration for serving simple webpages securely with luna.
// This app demonstrates, very briefly, how to serve secure endpoints with TLS
//
/////////////////

#include <string>
#include <iostream>
#include <luna/luna.h>

using namespace luna;

////
// Of course, you need some certificates. At the moment, the only built-in supported method is to read them from
// memory as character strings; and the server must be restarted if the values change. We'll look at improving
// this in the future, but for now this is how it goes.
//
// You will need both the private key and the public certificate, as here. This cert only works with `localhost` so
// don't use them in production ;)

const char* key_pem = R"key(
-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQC2WAYBOrf1oVsqSTDuV7PCmbFCfFDdUyceVvoRFYlfKVG3oqzk
ihj74Gz9QN9A5p343wMTPR8T6pUF2vlsnhhWjZWjRtl3Q1ByeR68p3pX1x/VukL6
DpN9/hPja9SQ0O0SN3+nnazUqOWx4xNubjE2qtliRBruYJBToweAumgIWwIDAQAB
AoGAMg27qD68HLssUVFCE0sT10llNUD22apnxJZPkrz8tpibC/Olzh5yeUHD6UkT
f7dOifB94ZwXmdn9Clxvt3c2aqUlban29JCVIv4jNn3xSyX7x1SGSYkVfmdZmx8N
kqJzc3Cd9qqThpICMZorrJgiunvjv2lPez/WYBfopo1HHbECQQDlBpq2xvIxrEB9
xpshG7g92PFC/cBRpBiSddj9SNTE51KFfbwWQXATvEeAuCvNDvt4DMGhf8uR1RQ3
Y1u1Jf+NAkEAy9HnTSdJyvNK1oOGHY21qmzeQdsf9kSVwyk0ZrzjzjvpBMdGDanu
pmXXnAb6g4two1Pv/usZ8AkH1WOPze+ZhwJBALt+9ahPJTZfU4jk0FmBjQstEQ8M
GKd91RQKYLOzLWVqD5fzwMjjl7LAgDU0Eb/V1S+KGfdyXSEttVhHAmxCCH0CQCcB
bqCHSkn6sXHIDdyOCBwrZYV45+77RSXqNssKjxYixP9TG4b15GQXzWFgvIR8JPw8
zgsc9wPG1qbxvM2OcvUCQFaH28XQNEMgo/BI2dFJ9vp8lkrO2mlUHTUjdswGbO7K
3qxwR1O70oBS1gcq/xmKI0VXS0ctKuhL8OcRybgaK7w=
-----END RSA PRIVATE KEY-----
)key";
const char* cert_pem = R"key(
-----BEGIN CERTIFICATE-----
MIIDhzCCAvCgAwIBAgIJAKgEdzixR0vmMA0GCSqGSIb3DQEBBQUAMIGKMQswCQYD
VQQGEwJVUzESMBAGA1UECBMJQ29uZnVzaW9uMQ4wDAYDVQQHEwVDaGFvczEQMA4G
A1UEChMHRGlzY29yZDENMAsGA1UECxMETm9wZTESMBAGA1UEAxMJbG9jYWxob3N0
MSIwIAYJKoZIhvcNAQkBFhNleGFtcGxlQGV4YW1wbGUuY29tMB4XDTE2MTEwMTE0
NDgyNFoXDTE2MTIwMTE0NDgyNFowgYoxCzAJBgNVBAYTAlVTMRIwEAYDVQQIEwlD
b25mdXNpb24xDjAMBgNVBAcTBUNoYW9zMRAwDgYDVQQKEwdEaXNjb3JkMQ0wCwYD
VQQLEwROb3BlMRIwEAYDVQQDEwlsb2NhbGhvc3QxIjAgBgkqhkiG9w0BCQEWE2V4
YW1wbGVAZXhhbXBsZS5jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBALZY
BgE6t/WhWypJMO5Xs8KZsUJ8UN1TJx5W+hEViV8pUbeirOSKGPvgbP1A30Dmnfjf
AxM9HxPqlQXa+WyeGFaNlaNG2XdDUHJ5HrynelfXH9W6QvoOk33+E+Nr1JDQ7RI3
f6edrNSo5bHjE25uMTaq2WJEGu5gkFOjB4C6aAhbAgMBAAGjgfIwge8wHQYDVR0O
BBYEFMfiri7IVgqvj0h9xbrK6WuZEBtaMIG/BgNVHSMEgbcwgbSAFMfiri7IVgqv
j0h9xbrK6WuZEBtaoYGQpIGNMIGKMQswCQYDVQQGEwJVUzESMBAGA1UECBMJQ29u
ZnVzaW9uMQ4wDAYDVQQHEwVDaGFvczEQMA4GA1UEChMHRGlzY29yZDENMAsGA1UE
CxMETm9wZTESMBAGA1UEAxMJbG9jYWxob3N0MSIwIAYJKoZIhvcNAQkBFhNleGFt
cGxlQGV4YW1wbGUuY29tggkAqAR3OLFHS+YwDAYDVR0TBAUwAwEB/zANBgkqhkiG
9w0BAQUFAAOBgQBVm72s0mcOPEWUIt4IjemgCsuWC8PV8Fkqg4htuAu/K4cRWhM9
x4gFn/plNA1iaaorcn/LLFt4jv6CTB0KbEZsMgC4I3oGWECamg7CyfXKacvg34Ft
T5NU9xUNbQugJdCMSm+5TLeU5UhsvGqxVDAcIr3w9Iwsyti9CzRs6TzUog==
-----END CERTIFICATE-----
)key";


// Here are our loggers, just like in the basic_webapp example
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


int main(void)
{
    // Set up the logging
    set_error_logger(ex_error_log);
    set_access_logger(ex_access_log);

    // Here we add debugging output, and we pass in the private key and the public certificate to luna
    // Notice that setting up TLS precludes the possibility of serving non-https webpages, unfortunately. This is a bug.
    server server{
            server::debug_output{true},
            server::https_mem_key{key_pem},
            server::https_mem_cert{cert_pem}
    };


    // from here, everything is the same!
    // try reaching:
    // https://localhost:8442/hello_world
    router router{"/"};
    router.handle_request(request_method::GET,
                          "/hello_world",
                          [](auto req) -> response
                              {
                                  return {"<h1>Hello, World!</h1>"};
                              });

    server.add_router(router);

    // Block until the server shuts down, which is never. We could also do other things with this thread as well.
    server.start(8443);
}