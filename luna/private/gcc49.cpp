//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//
// borrowed from G2log

#if GCC_VERSION < 409000

#include <chrono>
#include <string>

namespace std
{

typedef chrono::time_point<chrono::system_clock> system_time_point;

tm localtime(const time_t& time)
{
    tm tm_snapshot;
    localtime_r(&time, &tm_snapshot);
    return tm_snapshot;
}


// To simplify things the return value is just a string. I.e. by design!
string put_time(const tm* date_time, const char* c_time_format)
{
    const size_t size = 1024;
    char buffer[size];
    auto success = strftime(buffer, size, c_time_format, date_time);

    if (0 == success)
        return c_time_format;

    return buffer;
}


// extracting time_t from std:chrono for "now"
time_t systemtime_now()
{
    system_time_point system_now = chrono::system_clock::now();
    return chrono::system_clock::to_time_t(system_now);
}

} // namespace std

#endif