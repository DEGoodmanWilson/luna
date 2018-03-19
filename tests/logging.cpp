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

TEST(logging, error_logger_test)
{
    std::string log_str{"NOPE"};
    luna::log_level log_lvl{luna::log_level::FATAL};
    luna::set_error_logger([&](luna::log_level level, const std::string& message)
                     {
                         log_lvl = level;
                         log_str = message;
                     });

    luna::error_log(luna::log_level::DEBUG, "hello");
    ASSERT_EQ("hello", log_str);
    ASSERT_EQ(luna::log_level::DEBUG, log_lvl);

    luna::reset_error_logger();
}

TEST(logging, basic_formatting_test)
{
    std::string log_str;
    luna::set_access_logger([&](const luna::request &request, const luna::response &response)
                     {
                         std::stringstream message;
                         message << luna::to_string(request.method) << " " << request.path << " " << request.http_version;
                         log_str = message.str();
                     });

    luna::request req{std::chrono::system_clock::now(), std::chrono::system_clock::now(), "", luna::request_method::GET, "/", "HTTP/1.0", {}, {}, {}, ""};
    luna::response res{200, "OK"};
    luna::access_log(req, res);
    ASSERT_EQ("GET / HTTP/1.0", log_str);

    luna::reset_access_logger();
}

TEST(logging, basic_formatting_test_2)
{
    std::string log_str;
    luna::set_access_logger([&](const luna::request &request, const luna::response &response)
                     {
                         std::stringstream message;
                         message << luna::to_string(request.method) << " " << request.path << " " << request.http_version;
                         log_str = message.str();
                     });

    luna::request req{std::chrono::system_clock::now(), std::chrono::system_clock::now(), "", luna::request_method::GET, "/", "HTTP/1.0", {}, {}, {}, ""};
    luna::response res{200, "OK"};
    luna::access_log(req, res);
    ASSERT_EQ("GET / HTTP/1.0", log_str);

    luna::reset_access_logger();
}

TEST(access_logging, integration_test)
{
    std::string access_log_str;
    luna::set_access_logger([&](const luna::request &request, const luna::response &response)
                            {
                                std::stringstream message;
                                message << luna::to_string(request.method) << " " << request.path << " " << request.http_version;
                                access_log_str = message.str();
                            });


    std::string error_log_str;
    luna::log_level log_lvl{luna::log_level::INFO};
    luna::set_error_logger([&](luna::log_level level, const std::string& message)
                           {
                               error_log_str = message;
                           });

    luna::server server;

    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    server.start_async();

    ASSERT_EQ("Luna server created on port 8080", error_log_str);
    ASSERT_TRUE(access_log_str.empty());
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("GET /test HTTP/1.1", access_log_str);

    luna::reset_access_logger();
    luna::reset_error_logger();
}