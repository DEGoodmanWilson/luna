//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#include "config.h"
#include "types.h"

namespace luna
{

logger_cb logger_ = nullptr;

void set_logger(logger_cb logger)
{
    logger_ = logger;
}

void reset_logger()
{
    logger_ = nullptr;
}

} //namespace luna


std::string to_string(luna::log_level value)
{
    switch(value)
    {
        case luna::log_level::FATAL:
            return "FATAL";
        case luna::log_level::ERROR:
            return "ERROR";
        case luna::log_level::WARNING:
            return "WARNING";
        case luna::log_level::INFO:
            return "INFO";
        case luna::log_level::DEBUG:
        default:
            return "DEBUG";
    }
}