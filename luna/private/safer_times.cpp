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
//
// this code borrowed and modified from G2log, which is in the public domain
// https://github.com/KjellKod/g3log/blob/master/LICENSE
// https://kjellkod.wordpress.com/2013/01/22/exploring-c11-part-2-localtime-and-time-again/
//


#include "luna/private/safer_times.h"

namespace luna
{

std::string put_time(const tm *date_time, const char *c_time_format)
{
    const size_t size = 1024;
    char buffer[size];
    auto success = strftime(buffer, size, c_time_format, date_time);

    if (0 == success)
    {
        return c_time_format;
    }

    return buffer;
}

//tm localtime(const std::time_t &time)
//{
//    std::tm tm_snapshot;
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
//    localtime_s(&tm_snapshot, &time);
//#else
//    localtime_r(&time, &tm_snapshot); // POSIX
//#endif
//    return tm_snapshot;
//}

tm gmtime(const std::time_t &time)
{
    std::tm tm_snapshot;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
    gmtime_s(&tm_snapshot, &time);
#else
    gmtime_r(&time, &tm_snapshot); // POSIX
#endif
    return tm_snapshot;
}

} //namespace luna