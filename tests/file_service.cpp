//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(file_service, serve_file_404)
{
    luna::server server{};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar.txt"});
    ASSERT_EQ(404, res.status_code);
}

TEST(file_service, serve_text_file)
{
    luna::set_logger([](luna::log_level level, const std::string &message)
                   {
                       std::cout << to_string(level) << " " << message << std::endl;
                   });

    luna::server server{};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    luna::reset_logger();
}

TEST(file_service, serve_html_file)
{
    luna::set_logger([](luna::log_level level, const std::string &message)
                         {
                             std::cout << to_string(level) << " " << message << std::endl;
                         });

    luna::server server{};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=us-ascii", res.header["Content-Type"]);
    luna::reset_logger();
}