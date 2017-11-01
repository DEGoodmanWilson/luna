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

TEST(crashers, 1_logging)
{
    luna::set_error_logger([](luna::log_level level, const std::string &mesg)
                           {
                               std::cout << mesg << std::endl;
                           });

    luna::router router{"/"};

    router.handle_request(luna::request_method::GET, "/hello", [](auto req) -> luna::response
    {
        return {"hello"};
    });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/hello"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);

    luna::reset_error_logger();
}

TEST(crashers, 2_query_params_with_no_values)
{

    luna::router router{"/"};

    router.handle_request(luna::request_method::GET, "/hello", [](auto req) -> luna::response
    {
        EXPECT_EQ(2, req.params.size());
        EXPECT_EQ(1, req.params.count("key1"));
        EXPECT_EQ("", req.params.at("key1"));
        EXPECT_EQ(1, req.params.count("key2"));
        EXPECT_EQ("foo", req.params.at("key2"));
        return {"hello"};
    });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/hello"},
                        cpr::Parameters{{"key1", ""},
                                        {"key2", "foo"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(crashers, 2_query_params_with_no_values_post)
{
    luna::router router{"/"};

    router.handle_request(luna::request_method::POST, "/hello", [](auto req) -> luna::response
    {
        EXPECT_EQ(2, req.params.size());
        EXPECT_EQ(1, req.params.count("key1"));
        EXPECT_EQ("", req.params.at("key1"));
        EXPECT_EQ(1, req.params.count("key2"));
        EXPECT_EQ("foo", req.params.at("key2"));
        return {"hello"};
    });

    luna::server server;
    server.add_router(router);
    server.start_async();

    auto res = cpr::Post(cpr::Url{"http://localhost:8080/hello"},
                         cpr::Payload{{"key1", ""},
                                      {"key2", "foo"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}