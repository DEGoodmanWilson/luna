//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(config, log_string_literal_rendering)
{
    ASSERT_EQ("FATAL", to_string(luna::log_level::FATAL));
    ASSERT_EQ("ERROR", to_string(luna::log_level::ERROR));
    ASSERT_EQ("WARNING", to_string(luna::log_level::WARNING));
    ASSERT_EQ("INFO", to_string(luna::log_level::INFO));
    ASSERT_EQ("DEBUG", to_string(luna::log_level::DEBUG));
}

TEST(config, logging)
{
    std::string log;
    luna::set_logger([&log](luna::log_level level, const std::string &mesg)
                         {
                             log = "foobar";
                         });

    //spool up a server, which should generate an INFO log with the port
    luna::server server;

    ASSERT_EQ("foobar", log);
}