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


TEST(advanced_functioning, check_arbitrary_headers_case_insensitive)
{
    luna::server server;
    server.handle_request(luna::request_method::GET, "/test", [](const luna::request &req) -> luna::response
    {
        EXPECT_EQ(1, req.headers.count("heLLo"));
        EXPECT_EQ(1, req.headers.count("HELLO"));
        EXPECT_EQ(1, req.headers.count("hello"));
        EXPECT_EQ(0, req.headers.count("NOPE"));

        return {200, luna::request_headers{{"gOOdbye", "yes"}}};
    });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Header{{"heLLo", "yes"}});
    ASSERT_EQ(1, res.header.count("gOOdbye"));
    ASSERT_EQ(1, res.header.count("GOODBYE"));
    ASSERT_EQ(1, res.header.count("goodbye"));
    ASSERT_EQ(0, res.header.count("NOPE"));
}

TEST(advanced_functioning, check_request_handler_removal)
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

TEST(advanced_functioning, response_headers)
{
    luna::server server;
    server.handle_request(luna::request_method::GET, "/test", [](auto req) -> luna::response
        {
            EXPECT_EQ(1, req.headers.count("foo"));
            EXPECT_EQ("bar", req.headers.at("foo"));
            return {req.headers};
        });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Header{{"foo", "bar"}});
    ASSERT_EQ(1, res.header.count("foo"));
    ASSERT_EQ("bar", res.header.at("foo"));
}

//TODO NOTICE that this test behaves differently on Linux and non-Linux
TEST(advanced_functioning, use_epoll)
{
    luna::server server{luna::server::use_epoll_if_available{true}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(advanced_functioning, epoll_thread_per_connection_collision_1)
{
    bool got_log{false};

    luna::set_error_logger([&](luna::log_level level, const std::string &message)
                         {
                             if(message == "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_epoll_if_available" && level == luna::log_level::ERROR)
                             {
                                 got_log = true;
                             }
                         });

    luna::server server{luna::server::use_epoll_if_available{true}, luna::server::use_thread_per_connection{true}};

    ASSERT_TRUE(got_log);

    luna::reset_error_logger();
}

TEST(advanced_functioning, epoll_thread_per_connection_collision_2)
{
    bool got_log{false};

    luna::set_error_logger([&](luna::log_level level, const std::string &message)
                         {
                             if(message == "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_thread_per_connection" && level == luna::log_level::ERROR)
                             {
                                 got_log = true;
                             }
                         });

    luna::server server{luna::server::use_thread_per_connection{true}, luna::server::use_epoll_if_available{true}};

    ASSERT_TRUE(got_log);

    luna::reset_error_logger();
}

TEST(advanced_functioning, non_null_server_string_version)
{
    luna::server server;
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    std::string server_str="Luna/";
    ASSERT_NE("", LUNA_VERSION);
    ASSERT_NE(server_str, res.header["Server"]);
}

TEST(advanced_functioning, default_server_string)
{
    luna::server server;
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    std::string server_str{LUNA_NAME};
    server_str += "/";
    server_str += LUNA_VERSION;
    ASSERT_EQ(server_str, res.header["Server"]);
}

TEST(advanced_functioning, custom_server_string)
{
    luna::server server{luna::server::server_identifier{"foobar"}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("foobar", res.header["Server"]);
}

TEST(advanced_functioning, append_server_string)
{
    luna::server server{luna::server::append_to_server_identifier{"foobar"}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    std::string server_str{LUNA_NAME};
    server_str += "/";
    server_str += LUNA_VERSION;
    server_str += " foobar";
    ASSERT_EQ(server_str, res.header["Server"]);
}


TEST(file_service, check_paths_1)
{
    luna::server server{};

    server.handle_request(luna::request_method::GET,
                          "/.*",
                          [](auto req) -> luna::response
                          {
                              return {"first"};
                          });

    server.handle_request(luna::request_method::GET,
                          "/second",
                          [](auto req) -> luna::response
                          {
                              return {"second"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/first"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/second"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text); //YUP THAT'S RIGHT!
}

TEST(file_service, check_paths_2)
{
    luna::server server{};

    server.handle_request(luna::request_method::GET,
                          "/second",
                          [](auto req) -> luna::response
                          {
                              return {"second"};
                          });

    server.handle_request(luna::request_method::GET,
                          "/.*",
                          [](auto req) -> luna::response
                          {
                              return {"first"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/first"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/second"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("second", res.text); //YUP THAT'S RIGHT!
}


TEST(file_service, check_paths_4)
{
    luna::server server{};

    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");

    server.handle_request(luna::request_method::GET,
                          "/second",
                          [](auto req) -> luna::response
                          {
                              return {"second"};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/second"});
    ASSERT_EQ(404, res.status_code); //YUP THAT'S RIGHT!
}

TEST(file_service, check_paths_3)
{
    luna::server server{};


    server.handle_request(luna::request_method::GET,
                          "/first",
                          [](auto req) -> luna::response
                          {
                              return {"first"};
                          });

    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");


    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/first"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text);
}
