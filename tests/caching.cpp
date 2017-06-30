//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>
#include <array>
#include <thread>
#include <chrono>


TEST(cacheing, cache_read_1)
{
    std::shared_ptr<std::string> cache = std::make_shared<std::string>("hello");

    auto read = [&](const std::string &key) -> std::shared_ptr<std::string>
    {
        return cache;
    };

    luna::server server{luna::cache::build(read, nullptr)};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/foobar"});

    ASSERT_EQ("hello", res.text);
}

TEST(cacheing, cache_write_1)
{
    std::shared_ptr<std::string> cache;

    auto write = [&](const std::string &key, std::shared_ptr<std::string> value)
    {
        cache = value;
    };

    luna::server server{luna::cache::build(nullptr, write)};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});

    ASSERT_EQ("hello", res.text);
    // second call loads from cache
    cache = std::make_shared<std::string>("goodbye");
    res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_TRUE(static_cast<bool>(cache));
    ASSERT_EQ("hello", *cache);
}

TEST(cacheing, cache_read_write)
{
    std::shared_ptr<std::string> cache;

    bool cache_write{false};
    bool cache_hit{false};

    auto write = [&](const std::string &key, std::shared_ptr<std::string> value)
    {
        cache = value;
    };

    auto read = [&](const std::string &key) -> std::shared_ptr<std::string>
    {
        if (cache && !cache->empty())
        {
            cache_hit = true;
        }
        return cache;
    };

    luna::server server{luna::cache::build(read, write)};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");

    // first call loads from file and writes to cache
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});

    //TODO this is still indeterministic
    // add a delay to ensure cache write is complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ("hello", res.text);
    ASSERT_TRUE(static_cast<bool>(cache));
    ASSERT_EQ("hello", *cache);
    ASSERT_FALSE(cache_hit);

    // second call loads from cache
    cache = std::make_shared<std::string>("goodbye");
    res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ("goodbye", res.text);
    ASSERT_EQ("goodbye", *cache);
    ASSERT_TRUE(cache_hit);
}

TEST(cacheing, check_cache_threading)
{
    std::shared_ptr<std::string> cache;
    const std::thread::id original_thread{std::this_thread::get_id()};

    auto write = [&](const std::string &key, std::shared_ptr<std::string> value)
    {
        EXPECT_NE(original_thread, std::this_thread::get_id());
        cache = value;
        return true;
    };

    luna::server server{luna::cache::build(nullptr, write)};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ("hello", res.text);
    //TODO this is still indeterministic
    // add a delay to ensure cache write is complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(static_cast<bool>(cache));
    ASSERT_EQ("hello", *cache);
}

TEST(cacheing, check_cache_speedup)
{
    std::map<std::string, std::shared_ptr<std::string>> cache;
    int cache_writes{0};

    auto write = [&](const std::string &key, std::shared_ptr<std::string> value)
    {
        cache[key] = value;
        ++cache_writes;
        return true;
    };

    auto read = [&](const std::string &key) -> std::shared_ptr<std::string>
    {
        return cache[key];
    };

    //first, without cache
    const int total_times{100};

    std::chrono::high_resolution_clock::time_point t1, t2, t3, t4;
    {
        luna::server server;
        std::string path{std::getenv("STATIC_ASSET_PATH")};
        server.serve_files("/", path + "/tests/public");
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
    ASSERT_EQ(0, cache_writes);

    //second, with, cache
    {
        luna::server server{luna::cache::build(read, write)};
        std::string path{std::getenv("STATIC_ASSET_PATH")};
        server.serve_files("/", path + "/tests/public");

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

// This test make sure that cache writes can never happen after the server goes out of scope.
TEST(cacheing, cache_write_crasher_1)
{
    std::shared_ptr<std::string> cache;

    auto write = [&](const std::string &key, std::shared_ptr<std::string> value)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
        cache = value;
    };

    {
        luna::server server{luna::cache::build(nullptr, write)};
        std::string path{std::getenv("STATIC_ASSET_PATH")};
        server.serve_files("/", path + "/tests/public");

        auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
        ASSERT_EQ("hello", res.text);
    }

    ASSERT_TRUE(static_cast<bool>(cache));
    ASSERT_EQ("hello", *cache);
}

// This test make sure that cache writes can never happen after the server goes out of scope.
TEST(cacheing, cache_write_crasher_2)
{
    std::shared_ptr<std::string> cache;

    auto write = [&](const std::string &key, std::shared_ptr<std::string> value)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
        // if we aren't careful, this line will crash because cache will be out of scope!
        // The server should wait to halt until all cache writes are complete, to help novice programmers avoid this mistake.
        // This way, when the program exits, we won't get a segfault waiting on the last cache write operations to finish up.
        cache=value;
    };

    luna::server server{luna::cache::build(nullptr, write)};
    std::string path{std::getenv("STATIC_ASSET_PATH")};
    server.serve_files("/", path + "/tests/public");

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test.txt"});
    ASSERT_EQ("hello", res.text);
}