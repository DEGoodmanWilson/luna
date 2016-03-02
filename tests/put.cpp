//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(put, default_404)
{
    luna::server server{luna::server::port{8080}};
    auto res = cpr::Put(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(put, default_200)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::PUT,
                          "/test",
                          [](auto matches, auto params) -> luna::response
                              {
                                  return {"hello"};
                              });
    auto res = cpr::Put(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(put, default_200_check_params)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::PUT,
                          "/test",
                          [](auto matches, auto params) -> luna::response
                              {
                                  EXPECT_EQ(1, params.count("key"));
                                  EXPECT_EQ("value", params.at("key"));
                                  EXPECT_EQ(1, params.count("key2"));
                                  EXPECT_EQ("", params.at("key2"));
                                  return {"hello"};
                              });
    auto res = cpr::Put(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{{"key2", ""}, {"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}