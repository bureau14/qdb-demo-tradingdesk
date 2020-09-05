#include "utils/stringify.hpp"
#include <boost/predef/os/windows.h>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <utils/mktime.hpp>

namespace utils
{

using boost::spirit::karma::int_;
using boost::spirit::karma::right_align;

// YYYY-MM-DDTHH:MM:SS where T is the time separator
std::string to_iso_extended_string_utc(std::time_t t)
{
    std::tm time_struct;

#if BOOST_OS_WINDOWS
    gmtime_s(&time_struct, &t);
#else
    gmtime_r(&t, &time_struct);
#endif

    std::array<char, 32> local_buffer;
    local_buffer.back() = '\0';
    char * p            = std::data(local_buffer);

    if (!boost::spirit::karma::generate(p,
            (right_align(4, '0')[int_]                 //
                << '-' << right_align(2, '0')[int_]    //
                << '-' << right_align(2, '0')[int_] << // YYYY-MM-DD
                'T'                                    //
                << right_align(2, '0')[int_]           //
                << ':' << right_align(2, '0')[int_]    //
                << ':' << right_align(2, '0')[int_]    //
                << 'Z'),                               // HH:MM:SS Z indicates UTC
            time_struct.tm_year + 1900,                // years after 1900
            time_struct.tm_mon + 1,                    // [0-11] month
            time_struct.tm_mday,                       // [1-31] day of month
            time_struct.tm_hour,                       // [0-23] hours
            time_struct.tm_min,                        // [0-59] minutes
            time_struct.tm_sec)                        // [0-59] seconds
    )
    {
        return {};
    }

    p[0] = '\0';

    return {std::data(local_buffer), p};
}

std::chrono::system_clock::time_point from_iso_extended_string_utc(const std::string & str)
{
    using two_digits  = boost::spirit::qi::int_parser<int, 10, 2, 2>;
    using four_digits = boost::spirit::qi::int_parser<int, 10, 4, 4>;

    auto first = str.cbegin();
    auto last  = str.cend();

    std::tm time_struct;
    memset(&time_struct, 0, sizeof(time_struct));

    if (!boost::spirit::qi::parse(first, last,                              //
            four_digits() >> '-' >> two_digits() >> '-' >> two_digits() >>  // YYYY-MM-DD
                'T' >>                                                      //
                two_digits() >> ':' >> two_digits() >> ':' >> two_digits(), // HH:MM:SS
            time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday, time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec))
    {
        return std::chrono::system_clock::time_point();
    }

    time_struct.tm_year -= 1900; // years after 1900
    --time_struct.tm_mon;        // [0-11] month

    return std::chrono::system_clock::from_time_t(qdb::mkgmtime(&time_struct));
}

std::string to_simple_string(const std::chrono::system_clock::time_point & t)
{
    const std::time_t tt = std::chrono::system_clock::to_time_t(t);

    std::tm time_struct;
#if BOOST_OS_WINDOWS
    localtime_s(&time_struct, &tt);
#else
    localtime_r(&tt, &time_struct);
#endif

    std::array<char, 32> local_buffer;
    local_buffer.back() = '\0';
    char * p            = std::data(local_buffer);

    if (!boost::spirit::karma::generate(p,
            (right_align(4, '0')[int_]                 //
                << '-' << right_align(2, '0')[int_]    //
                << '-' << right_align(2, '0')[int_] << // YYYY-MM-DD
                ' '                                    //
                << right_align(2, '0')[int_]           //
                << ':' << right_align(2, '0')[int_]    //
                << ':' << right_align(2, '0')[int_]),  // HH:MM:SS
            time_struct.tm_year + 1900,                // years after 1900
            time_struct.tm_mon + 1,                    // [0-11] month
            time_struct.tm_mday,                       // [1-31] day of month
            time_struct.tm_hour,                       // [0-23] hours
            time_struct.tm_min,                        // [0-59] minutes
            time_struct.tm_sec)                        // [0-59] seconds
    )
    {
        return std::string();
    }

    p[0] = '\0';

    return {std::data(local_buffer), p};
}
} // namespace utils