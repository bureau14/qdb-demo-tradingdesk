#ifndef RXTERM_UTILS_HPP
#define RXTERM_UTILS_HPP

#include <boost/predef.h>
#include <algorithm>
#include <string>
#include <vector>

namespace rxterm
{

#if BOOST_OS_WINDOWS
#define ESC "\x1b"
#define CSI "\x1b["
#else
#define CSI "\e["
#endif

auto toString(std::string const & x) -> std::string
{
    return x;
}

template <class T>
auto toString(T const & x) -> decltype(std::to_string(x))
{
    return std::to_string(x);
}

std::vector<std::string> split(std::string const & str, const std::string & delimiter = "\n")
{
    std::vector<std::string> tokens;

    size_t start = 0U;
    auto end     = str.find(delimiter);
    while (end != std::string::npos)
    {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.size();
        end   = str.find(delimiter, start);
    }

    if (start != str.size()) tokens.push_back(str.substr(start, str.size() - start));

    return tokens;
}

template <class T, class F>
auto map(T const & data, F const & f)
{
    std::vector<decltype(f(data[0]))> result(data.size());
    std::transform(data.begin(), data.end(), result.begin(), f);
    return result;
}

std::string repeat(size_t n, std::string const & s)
{
    std::string result = "";
    for (unsigned i = 0; i < n; ++i)
    {
        result += s;
    }
    return result;
}

std::string clearBeforeCursor()
{
    return CSI "0K";
}

std::string clearAfterCursor()
{
    return CSI "1K";
}

std::string clearLine()
{
#if BOOST_OS_WINDOWS
    return CSI "2K";
#else
    return "\e[2K\r";
#endif
}

std::string turnOffCharAttributes()
{
    return CSI "0m";
}

std::string hideScreen()
{
    return CSI "0;8m";
}

std::string screenRendition(std::string r)
{
    return CSI + r + "m";
}

std::string moveUp(size_t n = 1)
{
#if BOOST_OS_WINDOWS
    return CSI + std::to_string(n) + "A";
#else
    return "\e[" + std::to_string(n) + "A\r";
#endif
}

#if BOOST_OS_WINDOWS
std::string clearLines(size_t n = 1)
{
    return CSI + std::string{"m"} + clearBeforeCursor() + ((n) ? repeat(n, clearLine() + moveUp()) : std::string(""));
}
#else
std::string clearLines(size_t n = 1)
{
    return "\e[0m" + clearBeforeCursor() + ((n) ? repeat(n, clearLine() + moveUp()) : std::string(""));
}
#endif

} // namespace rxterm

#endif
