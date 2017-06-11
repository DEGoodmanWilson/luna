//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>

TEST(logging, smoke_test)
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
    luna::set_access_logger([&](const luna::request &request)
                     {
                         std::stringstream message;
                         message << luna::to_string(request.method) << " " << request.path << " " << request.http_version;
                         log_str = message.str();
                     });

    luna::request req{std::chrono::system_clock::now(), std::chrono::system_clock::now(), "", luna::request_method::GET, "/", "HTTP/1.0", {}, {}, {}, ""};
    luna::access_log(req);
    ASSERT_EQ("GET / HTTP/1.0", log_str);
}

TEST(logging, basic_formatting_test_2)
{
    std::string log_str;
    luna::set_access_logger([&](const luna::request &request)
                     {
                         std::stringstream message;
                         message << luna::to_string(request.method) << " " << request.path << " " << request.http_version;
                         log_str = message.str();
                     });

    luna::request req{std::chrono::system_clock::now(), std::chrono::system_clock::now(), "", luna::request_method::GET, "/", "HTTP/1.0", {}, {}, {}, ""};
    luna::access_log(req);
    ASSERT_EQ("GET / HTTP/1.0", log_str);
}