#pragma once

#include <string>
#include <luna/luna.h>

void error_logger(luna::log_level level, const std::string &message);

void access_logger(const luna::request &request, const luna::response &response);