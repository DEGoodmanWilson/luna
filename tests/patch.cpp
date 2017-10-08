//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
//                   )
//
// Luna
// A web application and API framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(patch, default_404)
{

    luna::server server;
    server.start_async();

    auto res = cpr::Patch(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(patch, default_200)
{
    luna::router router;
    router.handle_request(luna::request_method::PATCH,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Patch(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(patch, default_200_check_params)
{
    luna::router router;
    router.handle_request(luna::request_method::PATCH,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  EXPECT_EQ(1, req.params.count("key"));
                                  EXPECT_EQ("value", req.params.at("key"));
                                  EXPECT_EQ(1, req.params.count("key2"));
                                  EXPECT_EQ("", req.params.at("key2"));
                                  return {"hello"};
                              });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Patch(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{{"key2", ""}, {"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}