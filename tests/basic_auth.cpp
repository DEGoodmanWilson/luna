//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <base64.h>

TEST(basic_auth, just_work)
{
    luna::headers header{{"Authorization", "Basic dXNlcjpwYXNz"}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_TRUE(static_cast<bool>(auth));
    ASSERT_EQ("user", auth.username);
    ASSERT_EQ("pass", auth.password);
}

TEST(basic_auth, just_work_edgecase)
{
    std::string userpass{"user:pass:pass"};
    luna::headers header{{"Authorization", "Basic " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_TRUE(static_cast<bool>(auth));
    ASSERT_EQ("user", auth.username);
    ASSERT_EQ("pass:pass", auth.password);
}


TEST(basic_auth, fail_1)
{
    std::string userpass{"user:pass"};
    luna::headers header{{"Heebiejeebie", "Basic " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, fail_2)
{
    std::string userpass{"user:pass"};
    luna::headers header{{"Authorization", "Basic waaah " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, fail_3)
{
    std::string userpass{"userpass"};
    luna::headers header{{"Authorization", "Basic " + base64_encode(userpass)}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}

TEST(basic_auth, fail_4)
{
    std::string userpass{"user:pass"};
    luna::headers header{{"Authorization", "Basic " + userpass}};
    auto auth = luna::get_basic_authorization(header);
    ASSERT_FALSE(static_cast<bool>(auth));
}