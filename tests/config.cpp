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
// Copyright © 2016–2018 D.E. Goodman-Wilson
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
    luna::set_error_logger([&log](luna::log_level level, const std::string& message)
                         {
                             log = "foobar";
                         });

    //spool up a server, which should generate an INFO log with the port
    luna::server server;
    server.start_async();

    ASSERT_EQ("foobar", log);

    luna::reset_error_logger();
}
