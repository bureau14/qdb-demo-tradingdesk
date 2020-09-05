#include <utils/mktime.hpp>
#include <ctime>
#include <string>

namespace qdb
{

static inline std::tm to_time_stamp(int year, int month, int day, int hour, int minute, int seconds) noexcept
{
    std::tm t;
    std::memset(&t, 0, sizeof(t));

    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = seconds;

    return t;
}

std::chrono::system_clock::time_point mktime(int year, int month, int day, int hour, int minute, int seconds) noexcept
{
    std::tm t = to_time_stamp(year, month, day, hour, minute, seconds);
    return std::chrono::system_clock::from_time_t(std::mktime(&t));
}

std::chrono::system_clock::time_point mktime_utc(int year, int month, int day, int hour, int minute, int seconds) noexcept
{
    std::tm t = to_time_stamp(year, month, day, hour, minute, seconds);

    return std::chrono::system_clock::from_time_t(mkgmtime(&t));
}

} // namespace qdb
