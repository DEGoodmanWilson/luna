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
// Copyright © 2016–2018 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(advanced_functioning, get_basic_regexes)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/([a-zA-Z0-9]*)",
                          [](auto req) -> luna::response
                          {
                              return {req.matches[1]};
                          });

    server.start_async();

    std::string path = "test";
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/" + path});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ(path, res.text);
}

TEST(advanced_functioning, get_basic_regexes_2)
{
    luna::server server;
    auto router = server.create_router("/api/v1");
    router->handle_request(luna::request_method::GET,
                          "/([a-zA-Z0-9]*)",
                          [](auto req) -> luna::response
                          {
                              return {req.matches[1]};
                          });

    server.start_async();

    std::string path = "test";
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/api/v1/" + path});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ(path, res.text);
}

TEST(advanced_functioning, post_basic_regexes)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::POST,
                          "/([a-zA-Z0-9]*)",
                          [](auto req) -> luna::response
                          {
                              return {req.matches[1]};
                          });

    server.start_async();

    std::string path = "test";
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/" + path},
                         cpr::Payload{}); //because posting requires an empty payload?
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ(path, res.text);
}

TEST(advanced_functioning, putting_it_together)
{
    std::set<std::string> docs;
    luna::server server;
    auto router = server.create_router("/");

    router->handle_request(luna::request_method::GET,
                          "/([a-zA-Z0-9]*)",
                          [&](auto req) -> luna::response
                          {
                              if (docs.count(req.matches[1])) return {req.matches[1]};

                              return {404};
                          });

    router->handle_request(luna::request_method::POST,
                          "/([a-zA-Z0-9]*)",
                          [&](auto req) -> luna::response
                          {
                              docs.emplace(req.matches[1]);
                              return {201};
                          });

    server.start_async();

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
    luna::server server;
    auto router = server.create_router("/");

    router->handle_request(luna::request_method::GET, "/redirect",
                          [](auto req) -> luna::response
                          {
                              return {301, luna::response::URI{"/foobar"}};
                          });

    router->handle_request(luna::request_method::GET, "/foobar",
                          [](auto req) -> luna::response
                          {
                              return {"bazqux"};
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/redirect"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("bazqux", res.text);
}

TEST(advanced_functioning, temporary_redirect)
{
    luna::server server;
    auto router = server.create_router("/");

    router->handle_request(luna::request_method::GET, "/redirect",
                          [](auto req) -> luna::response
                          {
                              return {307, luna::response::URI{"/foobar"}};
                          });

    router->handle_request(luna::request_method::GET, "/foobar",
                          [](auto req) -> luna::response
                          {
                              return {"bazqux"};
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/redirect"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("bazqux", res.text);
}

TEST(advanced_functioning, get_and_post)
{
    // POST params prevent GET params from being parsed. Weirdly. They just go away.
    luna::server server;
    auto router = server.create_router("/");

    router->handle_request(luna::request_method::POST,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              EXPECT_EQ(0, req.params.count("key1"));
                              EXPECT_EQ(1, req.params.count("key2"));
                              EXPECT_EQ("2", req.params.at("key2"));
                              return {"hello\n"};
                          });

    server.start_async();

    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test?key1=1"}, cpr::Payload{{"key2", "2"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello\n", res.text);
}

TEST(advanced_functioning, default_server_errors_404)
{
    luna::server server;

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ("<html><h1>404 Not Found</h1></html>", res.text);
}

TEST(advanced_functioning, default_server_errors_500)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {500, "Foobar"};
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("Foobar", res.text);
}

TEST(advanced_functioning, actual_server_errors)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              std::string{}.at(1); //throws out of bounds exception
                              return {}; //never hit
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("Internal error", res.text);
}

TEST(advanced_functioning, actual_server_errors2)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              throw new std::exception;
                              return {}; //never hit
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ("Unknown internal error", res.text);
}

//TODO custom error handlers

TEST(advanced_functioning, check_arbitrary_headers)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET, "/test", [](auto req) -> luna::response
    {
        EXPECT_EQ(1, req.headers.count("foo"));
        EXPECT_EQ("bar", req.headers.at("foo"));
        return {req.headers.at("foo")};
    });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Header{{"foo", "bar"}});
    ASSERT_EQ("bar", res.text);
}


TEST(advanced_functioning, check_arbitrary_headers_case_insensitive)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET, "/test", [](const luna::request &req) -> luna::response
    {
        EXPECT_EQ(1, req.headers.count("heLLo"));
        EXPECT_EQ(1, req.headers.count("HELLO"));
        EXPECT_EQ(1, req.headers.count("hello"));
        EXPECT_EQ(0, req.headers.count("NOPE"));

        return {200, luna::request_headers{{"gOOdbye", "yes"}}};
    });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Header{{"heLLo", "yes"}});
    ASSERT_EQ(1, res.header.count("gOOdbye"));
    ASSERT_EQ(1, res.header.count("GOODBYE"));
    ASSERT_EQ(1, res.header.count("goodbye"));
    ASSERT_EQ(0, res.header.count("NOPE"));
}

TEST(advanced_functioning, response_headers)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET, "/test", [](auto req) -> luna::response
    {
        EXPECT_EQ(1, req.headers.count("foo"));
        EXPECT_EQ("bar", req.headers.at("foo"));
        return {req.headers};
    });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Header{{"foo", "bar"}});
    ASSERT_EQ(1, res.header.count("foo"));
    ASSERT_EQ("bar", res.header.at("foo"));
}

//TODO NOTICE that this test behaves differently on Linux and non-Linux
TEST(advanced_functioning, use_epoll)
{
    luna::server server{luna::server::use_epoll_if_available{true}};
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello\n"};
                          });

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello\n", res.text);
}

TEST(advanced_functioning, epoll_thread_per_connection_collision_1)
{
    bool got_log{false};

    luna::set_error_logger([&](luna::log_level level, const std::string &message)
                           {
                               if (message ==
                                   "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_epoll_if_available" &&
                                   level == luna::log_level::ERROR)
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
                               if (message ==
                                   "Cannot combine use_thread_per_connection with use_epoll_if_available. Disabling use_thread_per_connection" &&
                                   level == luna::log_level::ERROR)
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
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello\n"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    std::string server_str = "Luna/";
    ASSERT_NE("", LUNA_VERSION);
    ASSERT_NE(server_str, res.header["Server"]);
}

TEST(advanced_functioning, default_server_string)
{
    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello\n"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    std::string server_str{LUNA_NAME};
    server_str += "/";
    server_str += LUNA_VERSION;
    ASSERT_EQ(server_str, res.header["Server"]);
}

TEST(advanced_functioning, custom_server_string)
{
    luna::server server{luna::server::server_identifier{"foobar"}};

    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello\n"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("foobar", res.header["Server"]);
}

TEST(advanced_functioning, custom_server_string_and_version)
{
    luna::server server{luna::server::server_identifier_and_version{"foobar", "1.0"}};

    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                           "/test",
                           [](auto req) -> luna::response
                           {
                               return {"hello\n"};
                           });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("foobar/1.0", res.header["Server"]);
}

TEST(advanced_functioning, append_server_string)
{
    luna::server server{luna::server::append_to_server_identifier{"foobar"}};

    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello\n"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    std::string server_str{LUNA_NAME};
    server_str += "/";
    server_str += LUNA_VERSION;
    server_str += " foobar";
    ASSERT_EQ(server_str, res.header["Server"]);
}


TEST(file_service, check_paths_1)
{

    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/.*",
                          [](auto req) -> luna::response
                          {
                              return {"first"};
                          });

    router->handle_request(luna::request_method::GET,
                          "/second",
                          [](auto req) -> luna::response
                          {
                              return {"second"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/first"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/second"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text); //YUP THAT'S RIGHT!
}

TEST(file_service, check_paths_2)
{

    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/second",
                          [](auto req) -> luna::response
                          {
                              return {"second"};
                          });

    router->handle_request(luna::request_method::GET,
                          "/.*",
                          [](auto req) -> luna::response
                          {
                              return {"first"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/first"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/second"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("second", res.text); //YUP THAT'S RIGHT!
}


TEST(file_service, check_paths_4)
{

    luna::server server;
    auto router = server.create_router("/");

    std::string path{STATIC_ASSET_PATH};
    router->serve_files("/", path + "/tests/public");


    router->handle_request(luna::request_method::GET,
                          "/second",
                          [](auto req) -> luna::response
                          {
                              return {"second"};
                          });


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello\n", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/second"});
    ASSERT_EQ(404, res.status_code); //YUP THAT'S RIGHT!
}

TEST(file_service, check_paths_3)
{


    luna::server server;
    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/first",
                          [](auto req) -> luna::response
                          {
                              return {"first"};
                          });

    std::string path{STATIC_ASSET_PATH};
    router->serve_files("/", path + "/tests/public");


    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello\n", res.text);

    res = cpr::Get(cpr::Url{"http://localhost:8080/first"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("first", res.text);
}

TEST(advanced_functioning, custom_not_found_renderer)
{
    luna::server server{
        [](const luna::request &req, luna::response &res)
{
            res.content = "NOPE!";
}
    };

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/missing"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ("NOPE!", res.text);
}

TEST(advanced_functioning, custom_not_found_renderer_file)
{
    luna::server server{
            [](const luna::request &req, luna::response &res)
            {
                res.content = "NOPE!";
            }
    };

    auto router = server.create_router("/");
    std::string path{STATIC_ASSET_PATH};
    router->serve_files("/", path + "/tests/public");

    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/missing"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ("NOPE!", res.text);
}
