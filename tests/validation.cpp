//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//

#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(validation, validate_match)
{
    auto v = luna::parameter::validate(luna::parameter::match, "value");
    ASSERT_TRUE(v("value"));
    ASSERT_FALSE(v("node"));
}

TEST(validation, number_match)
{
    auto v = luna::parameter::validate(luna::parameter::number);
    ASSERT_TRUE(v("0"));
    ASSERT_TRUE(v("10"));
    ASSERT_TRUE(v("01"));
    ASSERT_TRUE(v("00912321890412"));

    ASSERT_FALSE(v("0.4"));
    ASSERT_FALSE(v("10.1010"));
    ASSERT_FALSE(v(".1010"));
    ASSERT_FALSE(v("10."));

    ASSERT_FALSE(v("."));
    ASSERT_FALSE(v("0n"));
    ASSERT_FALSE(v("0n0"));
    ASSERT_FALSE(v("n0"));
    ASSERT_FALSE(v("10.10.10"));
    ASSERT_FALSE(v("value"));
}

TEST(validation, regex_match)
{
    auto v = luna::parameter::validate(luna::parameter::regex, std::regex{"[0-9]"});
    ASSERT_TRUE(v("0"));
    ASSERT_TRUE(v("10"));
    ASSERT_TRUE(v("01"));
    ASSERT_TRUE(v("0n"));
    ASSERT_TRUE(v("0n0"));
    ASSERT_TRUE(v("n0"));
    ASSERT_TRUE(v("10.10.10"));
    ASSERT_FALSE(v("value"));
}

TEST(validation, basic_validation_pass)
{
    luna::server server{luna::server::port{8080}};

    luna::parameter::validators validators = {
            {"key", luna::parameter::optional, luna::parameter::validate(luna::parameter::match, "value")},
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          validators);
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, basic_validation_fail)
{
    luna::server server{luna::server::port{8080}};

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::optional, luna::parameter::validate(luna::parameter::match,
                                                                                               "value")}
                          }
    );
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "nope"}});
    ASSERT_EQ(400, res.status_code);
}

TEST(validation, required_validation_pass)
{
    luna::server server{luna::server::port{8080}};

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required}
                          }
    );
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, required_validation_fail)
{
    luna::server server{luna::server::port{8080}};

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required}
                          }
    );
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(400, res.status_code);
}
