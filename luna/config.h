//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#pragma once

#include <string>
#include <functional>

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


using logger_cb = std::function<void(log_level level, const std::string &message)>;


void set_logger(logger_cb logger);
void reset_logger();


} //namespace luna

std::string to_string(luna::log_level value);