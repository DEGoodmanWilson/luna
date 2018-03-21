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

#include "luna/config.h"
#include "luna/types.h"
#include <sstream>
#include <regex>

std::string to_string(luna::log_level value)
{
    switch (value)
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

namespace luna
{

access_logger_cb access_logger_ = nullptr;

void set_access_logger(access_logger_cb access_logger)
{
    access_logger_ = access_logger;
}

void reset_access_logger()
{
    access_logger_ = nullptr;
}

error_logger_cb error_logger_ = nullptr;

void set_error_logger(error_logger_cb error_logger)
{
    error_logger_ = error_logger;
}

void reset_error_logger()
{
    error_logger_ = nullptr;
}


void access_log(const request &request, const response &response)
{
    if(access_logger_)
        access_logger_(request, response);
}

void error_log(luna::log_level level, const std::string &message)
{
    if(error_logger_)
        error_logger_(level, message);
}


} //namespace luna
