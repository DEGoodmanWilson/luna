//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#pragma once

#if GCC_VERSION < 409000

#include <string>
#include <chrono>

namespace std
{
string put_time(const tm *date_time, const char *c_time_format);
}
#endif