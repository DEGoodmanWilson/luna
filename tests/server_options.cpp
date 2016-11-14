//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>
#include <mutex>
#include <thread>
#include <chrono>
#include "server_impl.h"

using namespace std::chrono_literals;

TEST(server_options, set_mime_type)
{
    luna::server server{luna::server::mime_type{"howdyho"}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});

    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ("howdyho", res.header["Content-Type"]);

    //reset the default mime type
    // TODO find a better way to do this…
    luna::server s2{luna::server::mime_type{"text/html; charset=UTF-8"}};
}

TEST(server_options, set_error_handler_cb)
{
    luna::server server{luna::server::error_handler_cb
                                {
                                        [](luna::response &response, //a hook for modifying in place to insert default content
                                           luna::request_method method,
                                           const std::string &path)
                                            {
                                                ASSERT_EQ(404, response.status_code);
                                                ASSERT_EQ("/test", path);
                                                ASSERT_EQ(luna::request_method::GET, method);
                                            }
                                }
    };

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
}

TEST(server_options, set_accept_policy_cb)
{
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
    server.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ("", res.text);
}

TEST(server_options, set_unescaper_cb)
{
    luna::server server{luna::server::unescaper_cb
                                {
                                        [](const std::string &text) -> std::string
                                            {
                                                if (text == "value") return "ugh";
                                                return text;
                                            }
                                }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  EXPECT_EQ("ugh", req.params["key"]);
                                  return {req.params["key"]};
                              });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ("ugh", res.text);
}

TEST(server_options, set_thread_pool_size)
{
    uint8_t count = 0;
    uint8_t max_count = 0;
    std::mutex mutex;

    luna::server server{luna::server::thread_pool_size{5}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [&count, &max_count, &mutex](auto req) -> luna::response
                              {
                                  mutex.lock();
                                  ++count;
                                  if (count > max_count) max_count = count;
                                  mutex.unlock();

                                  std::this_thread::sleep_for(10ms);

                                  mutex.lock();
                                  --count;
                                  mutex.unlock();
                                  return {"Hello"};
                              });

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
    ASSERT_EQ(5, max_count);
}

TEST(server_options, use_thread_per_connection)
{
    uint8_t count = 0;
    uint8_t max_count = 0;
    std::mutex mutex;

    luna::server server{luna::server::use_thread_per_connection{true}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [&count, &max_count, &mutex](auto req) -> luna::response
                              {
                                  mutex.lock();
                                  ++count;
                                  if (count > max_count) max_count = count;
                                  mutex.unlock();

                                  std::this_thread::sleep_for(10ms);

                                  mutex.lock();
                                  --count;
                                  mutex.unlock();
                                  return {"Hello"};
                              });

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
    ASSERT_EQ(10, max_count);
}

TEST(server_options, set_connection_limit)
{
    uint8_t count = 0;
    uint8_t max_count = 0;
    std::mutex mutex;

    luna::server server{luna::server::connection_limit{2}, luna::server::thread_pool_size{5}};
    server.handle_request(luna::request_method::GET,
                          "/test",
                          [&count, &max_count, &mutex](auto req) -> luna::response
                              {
                                  mutex.lock();
                                  ++count;
                                  if (count > max_count) max_count = count;
                                  mutex.unlock();

                                  std::this_thread::sleep_for(10ms);

                                  mutex.lock();
                                  --count;
                                  mutex.unlock();
                                  return {"Hello"};
                              });

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
    ASSERT_EQ(2, max_count);
}