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
#include <mutex>
#include <thread>
#include <array>
#include <chrono>
#include "server_impl.h"

using namespace std::chrono_literals;

TEST(server_options, set_mime_type)
{
    luna::router router;
    router.set_mime_type("howdyho");

    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    luna::server server{};
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});

    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("howdyho", res.header["Content-Type"]);
}

TEST(server_options, set_accept_policy_cb)
{
    luna::router router{"/"};
    router.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });

    luna::server server{luna::server::accept_policy_cb
                                {
                                        [](const struct sockaddr *add, socklen_t len) -> bool
                                        {
                                            //we can't really know what address will get passed in so just assert that
                                            // returning "false" does the right thing, and that we got called.
                                            EXPECT_NE(nullptr, add);
                                            return false;
                                        }
                                }
    };
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("", res.text);
}

TEST(server_options, set_unescaper_cb)
{
    luna::router router{"/"};
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              EXPECT_EQ("ugh", req.params["key"]);
                              return {req.params["key"]};
                          });

    luna::server server{luna::server::unescaper_cb
                                {
                                        [](const std::string &text) -> std::string
                                        {
                                            if (text == "value") return "ugh";
                                            return text;
                                        }
                                }
    };
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ("ugh", res.text);
}

TEST(server_options, set_thread_pool_size)
{
    std::map<std::thread::id, uint16_t> thread_counter;
    std::mutex mutex;
    const int thread_pool_size{5};

    luna::router router{"/"};
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [&thread_counter, &mutex](auto req) -> luna::response
                          {
                              mutex.lock();
                              thread_counter[std::this_thread::get_id()] += 1;
                              mutex.unlock();

                              std::this_thread::sleep_for(100ms);

                              return {"Hello"};
                          });

    luna::server server{luna::server::thread_pool_size{thread_pool_size}};
    server.add_router(router);
    server.start_async();

    const int thread_count{50};
    std::array<std::thread, thread_count> threads;
    for (auto &t : threads)
    {
        t = std::thread{[]()
                        {
                            cpr::Get(cpr::Url{"http://localhost:8080/test"});
                        }};
    }

    for (auto &t : threads)
    {
        t.join();
    }

//    ASSERT_EQ(0, count);
    ASSERT_EQ(thread_pool_size, thread_counter.size());
}

TEST(server_options, use_thread_per_connection)
{
    std::map<std::thread::id, uint16_t> thread_counter;
    std::mutex mutex;

    luna::router router{"/"};
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [&thread_counter, &mutex](auto req) -> luna::response
                          {
                              mutex.lock();
                              thread_counter[std::this_thread::get_id()] += 1;
                              mutex.unlock();

                              std::this_thread::sleep_for(100ms);

                              return {"Hello"};
                          });

    luna::server server{luna::server::use_thread_per_connection{true}};
    server.add_router(router);
    server.start_async();

    const int thread_count{50};
    std::array<std::thread, thread_count> threads;
    for (auto &t : threads)
    {
        t = std::thread{[]()
                        {
                            cpr::Get(cpr::Url{"http://localhost:8080/test"});
                        }};
    }

    for (auto &t : threads)
    {
        t.join();
    }

    ASSERT_LT(1, thread_counter.size());
    ASSERT_GE(thread_count, thread_counter.size());
}

TEST(server_options, set_connection_limit)
{
    uint8_t count = 0;
    uint8_t max_count = 0;
    std::mutex mutex;

    luna::router router{"/"};
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [&count, &max_count, &mutex](auto req) -> luna::response
                          {
                              mutex.lock();
                              ++count;
                              if (count > max_count) max_count = count;
                              mutex.unlock();

                              std::this_thread::sleep_for(500ms);

                              mutex.lock();
                              --count;
                              mutex.unlock();
                              return {"Hello"};
                          });

    luna::server server{luna::server::connection_limit{2}, luna::server::thread_pool_size{5}};
    server.add_router(router);
    server.start_async();

    std::thread threads[10];
    for (int x = 0; x < 10; ++x)
    {
        threads[x] = std::thread{[]()
                                 {
                                     cpr::Get(cpr::Url{"http://localhost:8080/test"});
                                 }};
    }

    for (int x = 0; x < 10; ++x)
    {
        threads[x].join();
    }

    ASSERT_EQ(0, count);
    ASSERT_GE(2, max_count);
}