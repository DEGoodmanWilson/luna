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
    luna::server server{luna::server::mime_type{"howdyho"}
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

TEST(server_options, set_error_handler_cb)
{
    luna::server server{luna::server::error_handler_cb
                                {
                                        [](luna::response &response, //a hook for modifying in place to insert default content
                                           luna::request_method method,
                                           const std::string &path)
                                            {
                                                ASSERT_EQ(404, response.status_code);
                                                ASSERT_EQ("/test", path);
                                                ASSERT_EQ(luna::request_method::GET, method);
                                            }
                                }
    };

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
}

TEST(server_options, set_accept_policy_cb)
{
    luna::server server{luna::server::accept_policy_cb
                                {
                                        [](const struct sockaddr *add, socklen_t len) -> bool
                                            {
                                                //we can't really know what address will get passed in so just assert that
                                                // returning "false" does the right thing, and that we got called.
                                                EXPECT_NE(nullptr, add);
                                                return false;
                                            }
                                }
    };
    server.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto matches, auto params) -> luna::response
                              {
                                  return {"hello"};
                              });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("", res.text);
}

TEST(server_options, set_unescaper_cb)
{
    luna::server server{luna::server::unescaper_cb
                                {
                                        [](const std::string &text) -> std::string
                                            {
                                                if (text == "value") return "ugh";
                                                return text;
                                            }
                                }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto matches, auto params) -> luna::response
                              {
                                  EXPECT_EQ("ugh", params["key"]);
                                  return {params["key"]};
                              });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ("ugh", res.text);
}