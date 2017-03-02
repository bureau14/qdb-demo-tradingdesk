
#include "utils.hpp"

#include <boost/predef.h>

namespace utils
{

std::uint64_t muldiv64(std::uint64_t a, std::uint64_t b, std::uint64_t c)
{
#if BOOST_OS_WINDOWS
    static constexpr std::uint64_t base = 1ull << 32ull;
    static constexpr std::uint64_t maxdiv = (base - 1) * base + (base - 1);

    // First get the easy thing
    std::uint64_t res = (a / c) * b + (a % c) * (b / c);
    a %= c;
    b %= c;
    // Are we done?
    if (a == 0 || b == 0) return res;
    // Is it easy to compute what remain to be added?
    if (c < base) return res + (a * b / c);
    // Now 0 < a < c, 0 < b < c, c >= 1ULL
    // Normalize
    std::uint64_t norm = maxdiv / c;
    c *= norm;
    a *= norm;
    // split into 2 digits
    std::uint64_t ah = a / base, al = a % base;
    std::uint64_t bh = b / base, bl = b % base;
    std::uint64_t ch = c / base, cl = c % base;
    // compute the product
    std::uint64_t p0 = al * bl;
    std::uint64_t p1 = p0 / base + al * bh;
    p0 %= base;
    std::uint64_t p2 = p1 / base + ah * bh;
    p1 = (p1 % base) + ah * bl;
    p2 += p1 / base;
    p1 %= base;
    // p2 holds 2 digits, p1 and p0 one

    // first digit is easy, not null only in case of overflow
    // std::uint64_t q2 = p2 / c;
    p2 = p2 % c;

    // second digit, estimate
    std::uint64_t q1 = p2 / ch;
    // and now adjust
    std::uint64_t rhat = p2 % ch;
    // the loop can be unrolled, it will be executed at most twice for
    // even bases -- three times for odd one -- due to the normalisation above
    while (q1 >= base || (rhat < base && q1 * cl > rhat * base + p1))
    {
        q1--;
        rhat += ch;
    }
    // subtract
    p1 = ((p2 % base) * base + p1) - q1 * cl;
    p2 = (p2 / base * base + p1 / base) - q1 * ch;
    p1 = p1 % base + (p2 % base) * base;

    // now p1 hold 2 digits, p0 one and p2 is to be ignored
    std::uint64_t q0 = p1 / ch;
    rhat = p1 % ch;
    while (q0 >= base || (rhat < base && q0 * cl > rhat * base + p0))
    {
        q0--;
        rhat += ch;
    }
    // we do not need to do the subtraction (needed only to get the remainder,
    // in which case we have to divide it by norm)
    return res + q0 + q1 * base; // + q2 *base*base
#else
    return static_cast<std::uint64_t>(static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b) / static_cast<__uint128_t>(c));
#endif
}
    
}