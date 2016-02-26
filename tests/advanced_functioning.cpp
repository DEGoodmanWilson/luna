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
                          [](auto matches, auto params) -> luna::response
                              {
                                  return {matches[1]};
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
                          [](auto matches, auto params) -> luna::response
                              {
                                  return {matches[1]};
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
                          [&](auto matches, auto params) -> luna::response
                              {
                                  if (docs.count(matches[1])) return {matches[1]};

                                  return {404};
                              });
    server.handle_request(luna::request_method::POST,
                          "/([a-zA-Z0-9]*)",
                          [&](auto matches, auto params) -> luna::response
                              {
                                  docs.emplace(matches[1]);
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