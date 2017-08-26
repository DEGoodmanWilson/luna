//
//      _
//  ___/__)
// (, /      __   _
//   /   (_(_/ (_(_(_
//  (________________
//                   )
//
// Luna
// a web framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//

#pragma once


#include <chrono>
#include <string>

namespace luna
{
std::string put_time(const tm *date_time, const char *c_time_format);
//tm localtime(const time_t &time);
tm gmtime(const time_t &time);
}