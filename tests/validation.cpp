//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
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


TEST(validation, validate_any)
{
    auto v = luna::parameter::validate(luna::parameter::any);
    ASSERT_TRUE(v("value"));
    ASSERT_TRUE(v("nope"));
}

TEST(validation, validate_match)
{
    auto v = luna::parameter::validate(luna::parameter::match, "value");
    ASSERT_TRUE(v("value"));
    ASSERT_FALSE(v("nope"));
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
    luna::parameter::validators validators = {
            {"key", luna::parameter::optional, luna::parameter::validate(luna::parameter::match, "value")},
    };

    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          validators);

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, basic_validation_pass_with_lvalues)
{
    luna::parameter::validators validators{
            {"key", luna::parameter::optional, luna::parameter::validate(luna::parameter::match, "value")},
    };

    std::string endpoint{"/test"};

    luna::router router;
    router.handle_request(luna::request_method::GET,
                          endpoint,
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          validators);

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, basic_validation_fail)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
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

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "nope"}});
    ASSERT_EQ(400, res.status_code);
}

TEST(validation, required_validation_pass)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required}
                          }
    );

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, required_validation_pass_lvalue_1)
{
    const std::regex host_path{"/test"};

    luna::router router;
    router.handle_request(luna::request_method::GET,
                          host_path,
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required}
                          }
    );

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, required_validation_pass_lvalue_2)
{
    const std::regex host_path{"/test"};
    const luna::parameter::validators validators{
            {"key", luna::parameter::required}
    };

    luna::router router;
    router.handle_request(luna::request_method::GET,
                          host_path,
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          validators
    );

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, required_validation_fail)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required}
                          }
    );

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(400, res.status_code);
}


TEST(validation, custom_validation_pass)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required,
                                          luna::parameter::validate([](const std::string &a, int length) -> bool
                                                              {
                                                                  return a.length() <= length;
                                                              },
                                                              10)
                                  }
                          });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "0123456789"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(validation, custom_validation_fail)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          },
                          {
                                  {"key", luna::parameter::required,
                                          luna::parameter::validate([](const std::string &a, int length) -> bool
                                                              {
                                                                  return a.length() <= length;
                                                              },
                                                              10)
                                  }
                          });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "01234567890"}});
    ASSERT_EQ(400, res.status_code);
}
