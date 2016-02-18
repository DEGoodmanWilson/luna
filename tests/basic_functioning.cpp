//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(basic_functioning, just_work)
{
    luna::server server{luna::server::port{8080}};
    ASSERT_TRUE(static_cast<bool>(server)); //assert that the server is running
}
