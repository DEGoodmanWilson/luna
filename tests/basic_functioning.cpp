//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(basic_functioning, just_work)
{
    luna::server server{luna::server::port{8080}};
    ASSERT_TRUE(static_cast<bool>(server)); //assert that the server is running
}

TEST(basic_functioning, default_404)
{
    luna::server server{luna::server::port{8080}};
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(basic_functioning, default_200_with_get)
{
    luna::server server{luna::server::port{8080}};
    server.handle_response(luna::request_method::GET,
                           "/test",
                           [](std::vector<std::string> matches, luna::query_params params) -> luna::response
                               {
                                   return {"hello"};
                               });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(basic_functioning, default_200_with_get_check_params)
{
    luna::server server{luna::server::port{8080}};
    server.handle_response(luna::request_method::GET,
                           "/test",
                           [](std::vector<std::string> matches, luna::query_params params) -> luna::response
                               {
                                   EXPECT_EQ(1, params.count("key"));
                                   EXPECT_EQ("value", params.at("key"));
                                   return {"hello"};
                               });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(basic_functioning, default_201_with_post)
{
    luna::server server{luna::server::port{8080}};
    server.handle_response(luna::request_method::POST,
                           "/test",
                           [](std::vector<std::string> matches, luna::query_params params) -> luna::response
                               {
                                   return {"hello"};
                               });
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{{"key", "value"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}


TEST(basic_functioning, default_201_with_post_check_params)
{
    luna::server server{luna::server::port{8080}};
    server.handle_response(luna::request_method::POST,
                           "/test",
                           [](std::vector<std::string> matches, luna::query_params params) -> luna::response
                               {
                                   EXPECT_EQ(1, params.count("key"));
                                   EXPECT_EQ("value", params.at("key"));
                                   return {"hello"};
                               });
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{{"key", "value"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}