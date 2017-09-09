//
//      _
//  ___/__)
// (, /      __   _
//   /   (_(_/ (_(_(_
//  (________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(basic_functioning, just_work)
{
    luna::server server;
    server.start_async();
    ASSERT_TRUE(static_cast<bool>(server)); //assert that the server is running
    ASSERT_NE(0, static_cast<uint16_t>(server.get_port())); //assert that we have _some_ port, although none specified
}

TEST(basic_functioning, just_work_with_port)
{
    luna::server server;
    server.start_async();
    ASSERT_TRUE(static_cast<bool>(server)); //assert that the server is running
    ASSERT_EQ(8080, static_cast<uint16_t>(server.get_port()));

}

TEST(basic_functioning, just_work_with_specific_port)
{
    luna::server server;
    server.start_async(8081);
    ASSERT_TRUE(static_cast<bool>(server)); //assert that the server is running
    ASSERT_EQ(8081, static_cast<uint16_t>(server.get_port()));

}

TEST(basic_functioning, default_404)
{
    luna::server server;
    server.start_async();
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/"});
    ASSERT_EQ(404, res.status_code);
}

TEST(basic_functioning, default_200_with_get)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });

    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(basic_functioning, default_200_with_get_check_params)
{
    luna::router router;
    router.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  EXPECT_EQ(1, req.params.count("key"));
                                  EXPECT_EQ("value", req.params.at("key"));
                                  return {"hello"};
                              });
    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"}, cpr::Parameters{{"key", "value"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(basic_functioning, default_201_with_post)
{
    luna::router router;
    router.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });
    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{{"key", "value"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}


TEST(basic_functioning, default_201_with_post_check_params)
{
    luna::router router;
    router.handle_request(luna::request_method::POST,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  EXPECT_EQ(1, req.params.count("key"));
                                  EXPECT_EQ("value", req.params.at("key"));
                                  return {"hello"};
                              });
    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test"}, cpr::Payload{{"key", "value"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}


TEST(basic_functioning, default_201_with_post_json_in_body)
{
    luna::router router;
    router.handle_request(luna::request_method::POST, "/test", [](auto req) -> luna::response
        {
            EXPECT_EQ("{\"key\": \"value\"}", req.body);
            return {"hello"};
        });
    luna::server server;
    server.add_router(router);
    server.start_async();
    auto res = cpr::Post(cpr::Url{"http://localhost:8080/test"}, cpr::Body{"{\"key\": \"value\"}"}, cpr::Header{{"Content-Type", "application/json"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(basic_functioning, debug_logging)
{
    bool got_log{false};
    luna::set_error_logger([&](luna::log_level level, const std::string &message)
                         {
                             const std::string m{"Failed to bind to port 8080: Address already in use\n"};
                             if(message == m && level == luna::log_level::DEBUG)
                             {
                                 got_log = true;
                             }
                         });

    luna::server s1;
    s1.start_async();
    luna::server s2{luna::server::debug_output{true}};
    s2.start_async();

    ASSERT_TRUE(got_log);

    luna::reset_access_logger();
}

TEST(basic_functioning, async_start)
{
    luna::server server;
    ASSERT_FALSE(static_cast<bool>(server));
    server.start_async();
    ASSERT_TRUE(static_cast<bool>(server));
    server.stop();
    ASSERT_FALSE(static_cast<bool>(server));
}