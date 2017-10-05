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


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(headers, add_header)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });
    router.add_header("foo", "bar");

    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ(1, res.header.count("foo"));
    ASSERT_EQ("bar", res.header["foo"]);
}

TEST(headers, add_header_override)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              luna::response res{"hello"};
                              res.headers["foo"] = "nope";
                              return res;
                          });
    router.add_header("foo", "bar");

    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ(1, res.header.count("foo"));
    ASSERT_EQ("nope", res.header["foo"]);
}

TEST(headers, add_header_500)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              throw new std::exception;
                              return {}; //never hit
                          });

    router.add_header("foo", "bar");


    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ(1, res.header.count("foo"));
    ASSERT_EQ("bar", res.header["foo"]);
}
