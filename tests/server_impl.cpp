//
// luna
//
// Copyright © 2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <arpa/inet.h>

namespace luna
{
std::string addr_to_str_(const struct sockaddr *addr);
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