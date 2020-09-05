#ifndef RXTERM_TERMINAL_HPP
#define RXTERM_TERMINAL_HPP

#include <boost/predef.h>
#include <rxterm/utils.hpp>
#include <algorithm>
#include <iostream>
#include <string>

#if BOOST_OS_WINDOWS
#include <windows.h>
#endif

namespace rxterm
{

struct VirtualTerminal
{
    std::string buffer;

    std::string computeTransition(std::string const & next) const
    {
        if (buffer == next) return "";
        const auto n = std::count(buffer.begin(), buffer.end(), '\n');
        return clearLines(n) + turnOffCharAttributes() + next;
    }

    static std::string hide()
    {
        return hideScreen();
    }

    template <typename Handle>
    VirtualTerminal flip(Handle && h, std::string const & next) const
    {
        auto const transition = computeTransition(next);
        if (transition == "") return *this;

#if BOOST_OS_WINDOWS
        DWORD written = 0;

        ::WriteConsole(h, transition.data(), static_cast<DWORD>(transition.size()), &written, NULL);
#else
        h << transition << hide();
        std::flush(h);
#endif
        return {next};
    }
};

} // namespace rxterm

#endif
