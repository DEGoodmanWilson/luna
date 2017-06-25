//
// luna
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>
#include <array>
#include <thread>
#include <chrono>


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
    std::shared_ptr<std::string> cache = std::make_shared<std::string>("hello");

    luna::cache::read read = [&](const std::string &key) -> std::shared_ptr<std::string>
    {
        return cache;
    };

    luna::server server{luna::cache::build(read, nullptr)};
    server.serve_files("/", "../tests/public");
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar"});

    ASSERT_EQ("hello", res.text);
}

TEST(file_service, cache_write_1)
{
    std::shared_ptr<std::string> cache;

    luna::cache::write write = [&](const std::string &key, std::shared_ptr<std::string> value) -> bool
    {
        cache = value;
        return true;
    };

    luna::server server{luna::cache::build(nullptr, write)};
    server.serve_files("/", "../tests/public");

    ASSERT_EQ(nullptr, cache);

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});

    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("hello", *cache);
}

TEST(file_service, cache_read_write)
{
    std::shared_ptr<std::string> cache;
    bool cache_hit{false};

    luna::cache::write write = [&](const std::string &key, std::shared_ptr<std::string> value) -> bool
    {
        cache = value;
        return true;
    };

    luna::cache::read read = [&](const std::string &key) -> std::shared_ptr<std::string>
    {
        if (cache && !cache->empty())
        {
            cache_hit = true;
        }
        return cache;
    };

    luna::server server{luna::cache::build(read, write)};
    server.serve_files("/", "../tests/public");

    ASSERT_EQ(nullptr, cache);
    ASSERT_FALSE(cache_hit);


    // first call loads from file and writes to cache
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});

    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("hello", *cache);
    ASSERT_FALSE(cache_hit);

    // second call loads from cache
    cache = std::make_shared<std::string>("goodbye");
    res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ("goodbye", res.text);
    ASSERT_EQ("goodbye", *cache);
    ASSERT_TRUE(cache_hit);
}

TEST(file_service, check_cache_threading)
{
    std::shared_ptr<std::string> cache;
    const std::thread::id original_thread{std::this_thread::get_id()};

    luna::cache::write write = [&](const std::string &key, std::shared_ptr<std::string> value) -> bool
    {
        EXPECT_NE(original_thread, std::this_thread::get_id());
        cache = value;
        return true;
    };

    luna::server server{luna::cache::build(nullptr, write)};
    server.serve_files("/", "../tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("hello", *cache);
}

TEST(file_service, long_running_cache_writes)
{
    std::shared_ptr<std::string> cache;

    luna::cache::write write = [&](const std::string &key, std::shared_ptr<std::string> value) -> bool
    {
        //This sleep will ensure that the thread calling the cache write will outlive the server object.
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        cache = value;
        return true;
    };

    {
        luna::server server{luna::cache::build(nullptr, write)};
        server.serve_files("/", "../tests/public");

        ASSERT_EQ(nullptr, cache);

        auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
        ASSERT_EQ("hello", res.text);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    ASSERT_EQ("hello", *cache);
}

TEST(file_service, check_cache_speedup)
{
    std::map<std::string, std::shared_ptr<std::string>> cache;
    int cache_writes;

    luna::cache::write write = [&](const std::string &key, std::shared_ptr<std::string> value) -> bool
    {
        cache[key] = value;
        ++cache_writes;
        return true;
    };

    luna::cache::read read = [&](const std::string &key) -> std::shared_ptr<std::string>
    {
        return cache[key];
    };


    //first, without cache
    const int total_times{100};

    std::chrono::high_resolution_clock::time_point t1, t2, t3, t4;
    {
        luna::server server;
        server.serve_files("/", "../tests/public");
        t1 = std::chrono::high_resolution_clock::now();
        int times{total_times};
        while (times)
        {
            auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
            --times;
        }
        t2 = std::chrono::high_resolution_clock::now();
    }
    auto no_cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    std::cout << "No cacheing:   " << no_cache_duration << std::endl;


    //second, with, cache
    {
        luna::server server{luna::cache::build(read, write)};
        server.serve_files("/", "../tests/public");

        // load into cache
        cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});

        t3 = std::chrono::high_resolution_clock::now();
        int times{total_times};
        while (times)
        {
            auto res = cpr::Get(cpr::Url{"http://localhost:8080/nightmare.png"});
            --times;
        }
        t4 = std::chrono::high_resolution_clock::now();
    }
    auto cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    std::cout << "With cacheing: " << cache_duration << std::endl;
    ASSERT_EQ(1, cache_writes);
    ASSERT_LT(cache_duration, no_cache_duration);
}