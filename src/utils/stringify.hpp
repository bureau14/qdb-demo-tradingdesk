#pragma once

#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_int.hpp>
#include <boost/spirit/include/karma_uint.hpp>
#include <boost/spirit/include/qi_int.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <chrono>
#include <ctime>
#include <system_error>
#include <type_traits>
#if BOOST_OS_WINDOWS
#    ifndef VC_EXTRALEAN
#        define VC_EXTRALEAN
#    endif
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#endif

namespace utils
{

     std::string to_iso_extended_string_utc(std::time_t t);

    // YYYY-MM-DDTHH:MM:SS where T is the time separator
    static inline std::string to_iso_extended_string_utc(const std::chrono::system_clock::time_point& t)
    {
        return to_iso_extended_string_utc(std::chrono::system_clock::to_time_t(t));
    }

     std::chrono::system_clock::time_point from_iso_extended_string_utc(const std::string& str);

    // YYYY-MM-DD HH:MM:SS
     std::string to_simple_string(const std::chrono::system_clock::time_point& t);

namespace detail
{

// I could comment but I don't wanna
template <typename T, bool>
struct select_int_generator
{
    using type = boost::spirit::karma::int_generator<T>;
};

template <typename T>
struct select_int_generator<T, false>
{
    using type = boost::spirit::karma::uint_generator<T>;
};

template <typename T>
struct integer_generator
{
    using type = typename select_int_generator<T, std::is_signed_v<T>>::type;
};

template <typename T, bool>
struct select_int_parser
{
    using type = boost::spirit::qi::int_parser<T>;
};

template <typename T>
struct select_int_parser<T, false>
{
    using type = boost::spirit::qi::uint_parser<T>;
};

template <typename T>
struct integer_parser
{
    using type = typename select_int_parser<T, std::is_signed_v<T>>::type;
};

// very fast int to string and string to int converters (boost lexical cast is slow)
template <bool IncludeNullChar, typename Integer, typename StringLike>
void int_to_stringlike(Integer i, StringLike & local)
{
    static_assert(std::is_integral_v<Integer>, "only for integral types");

    // this will select the signed or unsigned *generator* automagically
    using integer_generator = typename detail::integer_generator<Integer>::type;

    // if you consider that 2^3 ~ 10^1, then you divide by three the number of bits to have the number of 10 based digits
    // note that dividing by 3 is generous, 2^32 is at maximum 9 figures large, not 10 or 11
    // the actual value ln(2)/ln(10) ~ 3.32
    //
    // max int is 2^64 that's about 10^20... 24 chars is more than enough if you count the sign and want to opt
    // for an easy to align value
    static constexpr size_t local_buffer_size = 24;

    // let's check at compile time nothing fancy happens (we might get a 128-bit integer, who knows!)
    static_assert(local_buffer_size > ((sizeof(Integer) / 3) + 1), "This integer type requires a larger buffer");
    local.resize(local_buffer_size);

    char * p = static_cast<char *>(local.data());

    const char * const start = p;
#ifndef NDEBUG
    const char * const end = p + local_buffer_size - 1;
#endif

    boost::spirit::karma::generate(p, integer_generator(), i);

    assert(p > start);
    assert(p < end);

    // ensure we have a 0, we have extra chars that make sure we can't overflow
    p[0] = '\0';

    if constexpr (IncludeNullChar)
    {
        ++p;
    }

    local.resize(p - start);
}

} // namespace detail

template <typename Integer>
void int_to_string(Integer i, std::string & local)
{
    detail::int_to_stringlike</*IncludeNullChar=*/false>(i, local);
}

template <typename Integer>
std::string int_to_string(Integer i)
{
    std::string local;
    int_to_string(i, local);
    return local;
}

template <typename Integer>
std::error_code int_from_string(const std::string_view & str, Integer & res)
{
    // this will select the signed or unsigned *parser* automagically
    using integer_parser = typename detail::integer_parser<Integer>::type;

    auto it   = str.cbegin();
    auto last = str.cend();

    const bool great_success = boost::spirit::qi::parse(it, last, integer_parser(), res);

    // we must have parsed successfully and consumed all the input
    return (great_success && (it == last)) ? std::error_code{} : std::make_error_code(std::errc::invalid_argument);
}

} // namespace utils
