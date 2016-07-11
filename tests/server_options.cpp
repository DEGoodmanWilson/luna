//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>
#include "server_impl.h"

TEST(server_options, set_mime_type)
{
    luna::server server{luna::server::port{8080},
                        luna::server::mime_type{"howdyho"}
    };
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto matches, auto params) -> luna::response
                              {
                                  return {"hello"};
                              });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});

    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("howdyho", res.header["Content-Type"]);
}

//This is going to be tough, as for the moment only indirect tests are possible. Would be nice to be able to sub
// in our own impl to make this work.