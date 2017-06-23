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
    server.serve_files("/", "../tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar.txt"});
    ASSERT_EQ(404, res.status_code);
}

TEST(file_service, serve_text_file)
{
    luna::server server{};
    server.serve_files("/", "../tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(file_service, serve_text_file2)
{
    luna::server server{};
    std::string mount{"/"};
    std::string filepath{"../tests/public"};
    server.serve_files(mount, filepath);

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(file_service, serve_html_file)
{
    luna::server server{};
    server.serve_files("/", "../tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=us-ascii", res.header["Content-Type"]);
}

TEST(file_service, serve_binary_file)
{
    luna::server server{};
    server.serve_files("/", "../tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/luna.jpg"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("image/jpeg; charset=binary", res.header["Content-Type"]);
    ASSERT_EQ(5196, res.text.size());
}

TEST(file_service, self_serve_html_file)
{
    luna::server server{};
    server.handle_request(luna::request_method::GET,
                          "/test.html",
                          [=](auto req) -> luna::response
                          {
                              std::string full_path = "../tests/public/test.html";
                              return luna::response::from_file(full_path);
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/html; charset=us-ascii", res.header["Content-Type"]);
}

TEST(file_service, self_serve_html_file_override_mime_type)
{
    luna::server server{};
    server.handle_request(luna::request_method::GET,
                          "/test.html",
                          [=](auto req) -> luna::response
                          {
                              std::string full_path = "../tests/public/test.html";
                              luna::response resp = luna::response::from_file(full_path);
                              resp.content_type = "text/foobar";
                              return resp;
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.html"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("text/foobar", res.header["Content-Type"]);
}

TEST(file_service, cache_read_1)
{
    luna::cache::read read = [](const std::string &key) -> std::string
    {
        return "hello";
    };

    luna::server server{luna::cache::build(read, nullptr)};
    server.serve_files("/", "../tests/public");
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar"});

    ASSERT_EQ("hello", res.text);
}

TEST(file_service, cache_write_1)
{
    std::string cache;

    luna::cache::write write = [&](const std::string &key, const std::string &value) -> bool
    {
        cache = value;
        return true;
    };

    luna::server server{luna::cache::build(nullptr, write)};
    server.serve_files("/", "../tests/public");

    ASSERT_EQ("", cache);

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});

    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("hello", cache);
}

TEST(file_service, cache_read_write)
{
    std::string cache;
    bool cache_hit{false};

    luna::cache::write write = [&](const std::string &key, const std::string &value) -> bool
    {
        cache = value;
        return true;
    };

    luna::cache::read read = [&](const std::string &key) -> std::string
    {
        if (!cache.empty())
        {
            cache_hit = true;
        }
        return cache;
    };

    luna::server server{luna::cache::build(read, write)};
    server.serve_files("/", "../tests/public");

    ASSERT_EQ("", cache);
    ASSERT_FALSE(cache_hit);


    // first call loads from file
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});

    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("hello", cache);
    ASSERT_FALSE(cache_hit);

    // second call loads from cache
    cache = "goodbye";
    res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ("goodbye", res.text);
    ASSERT_EQ("goodbye", cache);
    ASSERT_TRUE(cache_hit);
}