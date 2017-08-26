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

TEST(middleware, before_request_handler_middleware_modify_parameter)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a param to the query params list
            luna::middleware::before_request_handler{
                    [](luna::request &request)
                    {
                        request.params["boo"] = "scream";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              EXPECT_EQ(1, req.params.count("boo"));
                              EXPECT_EQ("scream", req.params.at("boo"));
                              return {"hello"};
                          });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(middleware, before_request_handler_middleware_modify_request_header)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::before_request_handler{
                    [](luna::request &request)
                    {
                        request.headers["boo"] = "scream";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              EXPECT_EQ(1, req.headers.count("boo"));
                              EXPECT_EQ("scream", req.headers.at("boo"));
                              return {"hello"};
                          });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(middleware, before_request_handler_middleware_modify_request_header_chained)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::before_request_handler{
                    [](luna::request &request)
                    {
                        request.headers["boo"] = "scream";
                    },
                    [](luna::request &request)
                    {
                        request.headers["ohno"] = "ohyes";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              EXPECT_EQ(1, req.headers.count("boo"));
                              EXPECT_EQ("scream", req.headers.at("boo"));
                              EXPECT_EQ(1, req.headers.count("ohno"));
                              EXPECT_EQ("ohyes", req.headers.at("ohno"));
                              return {"hello"};
                          });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(middleware, after_request_handler_middleware_modify_response_header)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::after_request_handler{
                    [](luna::response &response)
                    {
                        response.headers["boo"] = "scream";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ(1, res.header.count("boo"));
    ASSERT_EQ("scream", res.header.at("boo"));
}

TEST(middleware, after_request_handler_middleware_modify_response_header_chained)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::after_request_handler{
                    [](luna::response &response)
                    {
                        response.headers["boo"] = "scream";
                    },
                    [](luna::response &response)
                    {
                        response.headers["ohno"] = "ohyes";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {"hello"};
                          });
    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
    ASSERT_EQ(1, res.header.count("boo"));
    ASSERT_EQ("scream", res.header.at("boo"));
    ASSERT_EQ(1, res.header.count("ohno"));
    ASSERT_EQ("ohyes", res.header.at("ohno"));
}

TEST(middleware, after_request_handler_middleware_modify_response_header_fail_with_crash)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::after_request_handler{
                    [](luna::response &response)
                    {
                        response.headers["boo"] = "scream";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              std::string{}.at(1); //throws out of bounds exception
                              return {};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    // Assert that the header was not included in the output
    ASSERT_EQ(0, res.header.count("boo"));
}

TEST(middleware, after_error_middleware_modify_response_header_success)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::after_request_handler{
                    [](luna::response &response)
                    {
                        response.headers["boo1"] = "scream";
                    }
            },
            luna::middleware::after_error{
                    [](luna::response &response)
                    {
                        response.headers["boo2"] = "scream";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              std::string{}.at(1); //throws out of bounds exception
                              return {};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ(0, res.header.count("boo1"));
    ASSERT_EQ(1, res.header.count("boo2"));
    ASSERT_EQ("scream", res.header.at("boo2"));
}

TEST(middleware, after_error_middleware_modify_response_header_404_success)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::after_error{
                    [](luna::response &response)
                    {
                        response.headers["boo"] = "scream";
                    }
            }
    };

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(404, res.status_code);
    ASSERT_EQ(1, res.header.count("boo"));
    ASSERT_EQ("scream", res.header.at("boo"));
}

TEST(middleware, after_error_middleware_modify_response_header_500_success)
{
    luna::server server{
            luna::server::port{8080},
            //insert a simple middleware to append a header
            luna::middleware::after_request_handler{
                    [](luna::response &response)
                    {
                        response.headers["boo1"] = "scream";
                    }
            },
            luna::middleware::after_error{
                    [](luna::response &response)
                    {
                        response.headers["boo2"] = "scream";
                    }
            }
    };

    server.handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                          {
                              return {500};
                          });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/test"});
    ASSERT_EQ(500, res.status_code);
    ASSERT_EQ(1, res.header.count("boo1"));
    ASSERT_EQ("scream", res.header.at("boo1"));
    ASSERT_EQ(1, res.header.count("boo2"));
    ASSERT_EQ("scream", res.header.at("boo2"));
}