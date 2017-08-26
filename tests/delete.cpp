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

TEST(delete, default_404)
{
    luna::server server{luna::server::port{8080}};
    auto res = cpr::Delete(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(delete, default_200)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::DELETE,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });
    auto res = cpr::Delete(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}