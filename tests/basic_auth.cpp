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
#include <base64/base64.h>

TEST(basic_auth, helper_just_work)
{
    luna::headers header{{"Authorization", "Basic dXNlcjpwYXNz"}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_TRUE(static_cast<bool>(auth));
    ASSERT_EQ("user", auth.username);
    ASSERT_EQ("pass", auth.password);
}

TEST(basic_auth, helper_just_work_edgecase)
{
    std::string userpass{"user:pass:pass"};
    luna::headers header{{"Authorization", "Basic " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_TRUE(static_cast<bool>(auth));
    ASSERT_EQ("user", auth.username);
    ASSERT_EQ("pass:pass", auth.password);
}


TEST(basic_auth, helper_fail_1)
{
    std::string userpass{"user:pass"};
    luna::headers header{{"Heebiejeebie", "Basic " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, helper_fail_2)
{
    std::string userpass{"user:pass"};
    luna::headers header{{"Authorization", "Basic waaah " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, helper_fail_3)
{
    std::string userpass{"userpass"};
    luna::headers header{{"Authorization", "Basic " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, helper_fail_4)
{
    std::string userpass{"user:pass"};
    luna::headers header{{"Authorization", "Basic " + userpass}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, work_with_auth)
{
    std::string username{"foo"}, password{"bar"};

    luna::server server;
    auto router{server.create_router("/")};
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [=](auto req) -> luna::response
                              {
                                  auto auth = luna::get_basic_authorization(req.headers);

                                  EXPECT_TRUE(static_cast<bool>(auth));
                                  if(!auth || username != auth.username || password != auth.password) return luna::unauthorized_response{"realm"};

                                  return {"hello"};
                              });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Authentication{username, password});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(basic_auth, fail_with_401_no_auth_header)
{
    std::string username{"foo"}, password{"bar"};

    luna::server server;
    auto router{server.create_router("/")};
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [=](auto req) -> luna::response
                              {
                                  auto auth = luna::get_basic_authorization(req.headers);

                                  EXPECT_FALSE(static_cast<bool>(auth));
                                  if(!auth) return luna::unauthorized_response{"auth"};
                                  if(username != auth.username) return luna::unauthorized_response{"username"};
                                  if(password != auth.password) return luna::unauthorized_response{"password"};

                                  return {"hello"};
                              });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(401, res.status_code);
    ASSERT_EQ("Basic realm=\"auth\"", res.header["WWW-Authenticate"]);
}

TEST(basic_auth, fail_with_401_baduser)
{
    std::string username{"foo"}, password{"bar"};

    luna::server server;
    auto router{server.create_router("/")};
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [=](auto req) -> luna::response
                              {
                                  auto auth = luna::get_basic_authorization(req.headers);

                                  EXPECT_TRUE(static_cast<bool>(auth));
                                  if(!auth) return luna::unauthorized_response{"auth"};
                                  if(username != auth.username) return luna::unauthorized_response{"username"};
                                  if(password != auth.password) return luna::unauthorized_response{"password"};

                                  return {"hello"};
                              });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Authentication{"NOPE", password});
    ASSERT_EQ(401, res.status_code);
    ASSERT_EQ("Basic realm=\"username\"", res.header["WWW-Authenticate"]);
}

TEST(basic_auth, fail_with_401_badpass)
{
    std::string username{"foo"}, password{"bar"};

    luna::server server;
    auto router{server.create_router("/")};
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [=](auto req) -> luna::response
                              {
                                  auto auth = luna::get_basic_authorization(req.headers);

                                  EXPECT_TRUE(static_cast<bool>(auth));
                                  if(!auth) return luna::unauthorized_response{"auth"};
                                  if(username != auth.username) return luna::unauthorized_response{"username"};
                                  if(password != auth.password) return luna::unauthorized_response{"password"};

                                  return {"hello"};
                              });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Authentication{username, "NOPE"});
    ASSERT_EQ(401, res.status_code);
    ASSERT_EQ("Basic realm=\"password\"", res.header["WWW-Authenticate"]);
}