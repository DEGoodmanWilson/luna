//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

// At the moment these crash. Why?
//TEST(options, default_404)
//{
//    luna::server server{luna::server::port{8080}};
//    auto res = cpr::Options(cpr::Url{"http://localhost:8080/"});
//    ASSERT_EQ(404, res.status_code);
//}
//
//TEST(options, default_200)
//{
//    luna::server server{luna::server::port{8080}};
//    server.handle_request(luna::request_method::OPTIONS,
//                          "/test",
//                          [](auto req) -> luna::response
//                          {
//                              return {"hello"};
//                          });
//    auto res = cpr::Options(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{});
//    ASSERT_EQ(200, res.status_code);
//    ASSERT_EQ("hello", res.text);
//}