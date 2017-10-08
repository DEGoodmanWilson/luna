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
#include <luna/private/safer_times.h>
#include <iostream>
#include <iomanip>
#include <ctime>

TEST(safer_times, put_time_1)
{
    std::time_t t{0};
    auto tm = std::gmtime(&t);
    auto time_string = luna::put_time(tm, "%F %T");
    ASSERT_EQ("1970-01-01 00:00:00", time_string);
}

TEST(safer_times, put_time_2)
{
    std::time_t t{0};
    auto tm = std::gmtime(&t);
    auto time_string = luna::put_time(tm, "hello");
    ASSERT_EQ("hello", time_string);
}

TEST(safer_times, put_time_3)
{
    std::time_t t{0};
    auto tm = std::gmtime(&t);
    auto time_string = luna::put_time(tm, "");
    ASSERT_EQ("", time_string);
}