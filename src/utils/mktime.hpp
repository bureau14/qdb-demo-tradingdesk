#pragma once

#include <boost/predef/os/windows.h>
#include <chrono>
#include <ctime>

namespace qdb
{

// build a system clock from an UTC date or a date and time
std::chrono::system_clock::time_point mktime(int year, int month, int day, int hour, int minute, int seconds) noexcept;

inline std::chrono::system_clock::time_point mktime(int year, int month, int day) noexcept
{
    return mktime(year, month, day, 0, 0, 0);
}

std::chrono::system_clock::time_point mktime_utc(int year, int month, int day, int hour, int minute, int seconds) noexcept;

inline std::chrono::system_clock::time_point mktime_utc(int year, int month, int day) noexcept
{
    return mktime_utc(year, month, day, 0, 0, 0);
}

// Used in qdb_version
// Avoid linking utils into qdb_version
inline std::time_t mkgmtime(std::tm * t) noexcept
{
#if BOOST_OS_WINDOWS
    return _mkgmtime(t);
#else
    return ::timegm(t);
#endif
}

} // namespace qdb
