//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <arpa/inet.h>
#include <future>

namespace luna
{
std::string addr_to_str_(const struct sockaddr *addr);
request_method method_str_to_enum_(const std::string &method_str);
bool is_redirect_(status_code code);
}



TEST(server_impl, addr_to_string_with_null)
{
    auto res = luna::addr_to_str_(nullptr);
    ASSERT_EQ("", res);
}

TEST(server_impl, ip_address_rendering)
{
    std::string addr_str{"46.218.45.195"};
    struct sockaddr addr;
    int s = inet_pton(AF_INET, addr_str.c_str(), &addr);

    ASSERT_TRUE(s);
    ASSERT_EQ(addr_str, luna::addr_to_str_(&addr));
}

TEST(server_impl, is_redirect_)
{
    ASSERT_TRUE(luna::is_redirect_(301));
    ASSERT_FALSE(luna::is_redirect_(200));
    ASSERT_FALSE(luna::is_redirect_(400));
}



TEST(server_impl, method_str_to_enum)
{
    ASSERT_EQ(luna::request_method::GET, luna::method_str_to_enum_("GET"));
    ASSERT_EQ(luna::request_method::GET, luna::method_str_to_enum_("GET"));
    ASSERT_EQ(luna::request_method::POST, luna::method_str_to_enum_("POST"));
    ASSERT_EQ(luna::request_method::PUT, luna::method_str_to_enum_("PUT"));
    ASSERT_EQ(luna::request_method::PATCH, luna::method_str_to_enum_("PATCH"));
    ASSERT_EQ(luna::request_method::DELETE, luna::method_str_to_enum_("DELETE"));
    ASSERT_EQ(luna::request_method::OPTIONS, luna::method_str_to_enum_("OPTIONS"));
    ASSERT_EQ(luna::request_method::UNKNOWN, luna::method_str_to_enum_("WAT"));
}

// check that await actually works, and that we can shut down a server.
TEST(server_impl, await)
{
    luna::server server{luna::server::port{8080}};

    auto future = std::async(
            std::launch::async, [&server]() {
                EXPECT_TRUE(static_cast<bool>(server));
                server.await();
                EXPECT_FALSE(static_cast<bool>(server));
            });
    EXPECT_TRUE(future.wait_for(std::chrono::milliseconds(1000)) == std::future_status::timeout);
    server.stop();
    EXPECT_FALSE(future.wait_for(std::chrono::milliseconds(1000)) == std::future_status::timeout);
}
