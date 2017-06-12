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