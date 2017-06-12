//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(customization, custom_404)
{
    luna::server server{luna::server::port{8080}};
    server.handle_404([](const auto &req, auto &res)
                      {
                          res.content = "hello";
                      });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(customization, remove_custom_404)
{
    luna::server server{luna::server::port{8080}};
    auto handle = server.handle_404([](const auto &req, auto &res)
                      {
                          res.content = "hello";
                      });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ("hello", res.text);

    server.remove_error_handler(handle);

    res = cpr::Get(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ("<h1>Not found</h1>", res.text);
}

TEST(customization, custom_500)
{
    luna::server server{luna::server::port{8080}};
    server.handle_error(500, [](const auto &req, auto &res)
                      {
                          res.content = "hello";
                      });

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              std::string{}.at(1); //throws out of bounds exception
                              return {}; //never hit
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("hello", res.text);
}