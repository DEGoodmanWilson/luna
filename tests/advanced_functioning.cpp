//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(advanced_functioning, get_basic_regexes)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::GET,
                          "/([a-zA-Z0-9]*)",
                          [](auto req) -> luna::response
                              {
                                  return {req.matches[1]};
                              });

    std::string path = "test";
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/" + path});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ(path, res.text);
}

TEST(advanced_functioning, post_basic_regexes)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::POST,
                          "/([a-zA-Z0-9]*)",
                          [](auto req) -> luna::response
                              {
                                  return {req.matches[1]};
                              });

    std::string path = "test";
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/" + path},
                         cpr::Payload{}); //because posting requires an empty payload?
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ(path, res.text);
}

TEST(advanced_functioning, putting_it_together)
{
    luna::server server{luna::server::port{8080}};
    std::set<std::string> docs;
    server.handle_request(luna::request_method::GET,
                          "/([a-zA-Z0-9]*)",
                          [&](auto req) -> luna::response
                              {
                                  if (docs.count(req.matches[1])) return {req.matches[1]};

                                  return {404};
                              });
    server.handle_request(luna::request_method::POST,
                          "/([a-zA-Z0-9]*)",
                          [&](auto req) -> luna::response
                              {
                                  docs.emplace(req.matches[1]);
                                  return {201};
                              });

    std::string path{"foobar"};
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/" + path});
    ASSERT_EQ(404, res.status_code);
    res = cpr::Post(cpr::Url{"http://localhost:8080/" + path},
                    cpr::Payload{}); //because posting requires an empty payload?
    ASSERT_EQ(201, res.status_code);
    res = cpr::Get(cpr::Url{"http://localhost:8080/" + path});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ(path, res.text);
}

TEST(advanced_functioning, permanent_redirect)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::GET, "/redirect",
                          [](auto req) -> luna::response
                              {
                                  return {301, luna::response::URI{"/foobar"}};
                              });
    server.handle_request(luna::request_method::GET, "/foobar",
                          [](auto req) -> luna::response
                              {
                                  return {"bazqux"};
                              });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/redirect"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("bazqux", res.text);
}

TEST(advanced_functioning, temporary_redirect)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::GET, "/redirect",
                          [](auto req) -> luna::response
                              {
                                  return {307, luna::response::URI{"/foobar"}};
                              });
    server.handle_request(luna::request_method::GET, "/foobar",
                          [](auto req) -> luna::response
                              {
                                  return {"bazqux"};
                              });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/redirect"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("bazqux", res.text);
}

TEST(advanced_functioning, get_and_post)
{
    // POST params prevent GET params from being parsed. Weirdly. They just go away.
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  EXPECT_EQ(0, req.params.count("key1"));
                                  EXPECT_EQ(1, req.params.count("key2"));
                                  EXPECT_EQ("2", req.params.at("key2"));
                                  return {"hello"};
                              });
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test?key1=1"}, cpr::Payload{{"key2", "2"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(advanced_functioning, default_server_errors)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {500};
                              });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("<h1>So sorry, generic server error</h1>", res.text);
}

TEST(advanced_functioning, actual_server_errors)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  std::string{}.at(1); //throws out of bounds exception
                                  return {}; //never hit
                              });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("Internal error", res.text);
}

TEST(advanced_functioning, actual_server_errors2)
{
    luna::server server{luna::server::port{8080}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  throw new std::exception;
                                  return {}; //never hit
                              });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("Unknown internal error", res.text);
}

TEST(advanced_functioning, check_arbitrary_headers)
{
    luna::server server;
    server.handle_request(luna::request_method::GET, "/test", [](auto req) -> luna::response
        {
            EXPECT_EQ(1, req.headers.count("foo"));
            EXPECT_EQ("bar", req.headers.at("foo"));
            return {req.headers.at("foo")};
           });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Header{{"foo", "bar"}});
    ASSERT_EQ("bar", res.text);
}

TEST(advanced_functioning, check_response_handler_removal)
{
    luna::server server;
    auto handler = server.handle_request(luna::request_method::GET, "/test", [](auto req) -> luna::response
        {
           return {"foo"};
        });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("foo", res.text);
    server.remove_request_handler(handler);
    res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(404, res.status_code);
}