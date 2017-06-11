//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#pragma once

#include <string>
#include <functional>
#include <luna/types.h>

namespace luna
{

enum class log_level
{
    FATAL = 0,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
};


using access_logger_cb = std::function<void(const request &request)>;
using error_logger_cb = std::function<void(log_level level, const std::string &string)>;

void set_access_logger(access_logger_cb logger);
void reset_access_logger();

void set_error_logger(error_logger_cb logger);
void reset_error_logger();

void access_log(const luna::request& request);
void error_log(log_level level, const std::string &string);

} //namespace luna

std::string to_string(luna::log_level value);