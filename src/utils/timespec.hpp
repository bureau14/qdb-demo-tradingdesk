#pragma once

#include <qdb/client.h>
#include <boost/fusion/include/adapt_struct.hpp>
#include <cassert>
#include <chrono>
#include <cstdint>

namespace utils
{

namespace chr = std::chrono;

using seconds = chr::duration<std::int64_t>;
using milliseconds = chr::duration<std::int64_t, std::milli>;
using microseconds = chr::duration<std::int64_t, std::micro>;
using nanoseconds = chr::duration<std::int64_t, std::nano>;

// a cross platform time spec
// because there is no way to predict the underlying types of ::timespec
struct timespec
{
    constexpr timespec() noexcept
    {
    }

    constexpr timespec(seconds s, nanoseconds ns) noexcept : sec{s}, nsec{ns}
    {
    }

    constexpr explicit timespec(const qdb_timespec_t & ts) noexcept : sec{seconds(ts.tv_sec)}, nsec{nanoseconds(ts.tv_nsec)}
    {
    }

    constexpr explicit timespec(const ::timespec & ts) noexcept : sec{seconds(ts.tv_sec)}, nsec{nanoseconds(ts.tv_nsec)}
    {
    }

    template <typename Rep, typename Period>
    explicit timespec(chr::duration<Rep, Period> duration)
        : sec{chr::duration_cast<seconds>(duration)}, nsec{nanoseconds(duration % seconds(1))}
    {
        assert(nsec < seconds(1));
    }

    explicit constexpr operator bool() const noexcept
    {
        return (sec != seconds::zero()) || (nsec != nanoseconds::zero());
    }

    void reset() noexcept
    {
        sec = seconds::zero();
        nsec = nanoseconds::zero();
    }

    constexpr qdb_timespec_t as_timespec() const noexcept
    {
        return qdb_timespec_t{
            static_cast<decltype(qdb_timespec_t::tv_sec)>(sec.count()), static_cast<decltype(qdb_timespec_t::tv_nsec)>(nsec.count())};
    }

    // useful for API calls which are in ms
    constexpr std::uint64_t as_ms_timestamp() const noexcept
    {
        return static_cast<std::uint64_t>(
            (chr::duration_cast<milliseconds>(nanoseconds(nsec)) + chr::duration_cast<milliseconds>(seconds(sec))).count());
    }

    inline timespec & operator+=(nanoseconds duration) noexcept
    {
        auto new_nsec = nsec + duration;

        const auto dur_sec = chr::duration_cast<seconds>(new_nsec);
        new_nsec -= chr::duration_cast<nanoseconds>(dur_sec);

        sec += dur_sec;
        nsec = new_nsec;

        return *this;
    }

    timespec & operator-=(nanoseconds duration) noexcept
    {
        const auto dur_sec = chr::duration_cast<seconds>(duration);

        if (dur_sec > sec)
        {
            reset();
            return *this;
        }

        sec -= dur_sec;
        duration -= dur_sec;

        const auto dur_nsec = duration;
        assert(dur_nsec < seconds(1));

        if (dur_nsec > nsec)
        {
            if (sec == seconds::zero())
            {
                reset();
                return *this;
            }

            assert(sec >= seconds(1));

            --sec;
            nsec += seconds(1);
        }

        assert(nsec >= dur_nsec);
        nsec -= dur_nsec;
        assert(nsec < seconds(1));

        return *this;
    }

    constexpr bool operator<(const utils::timespec & right) const noexcept
    {
        return (sec < right.sec) || ((sec == right.sec) && (nsec < right.nsec));
    }

    constexpr bool operator>(const utils::timespec & right) const noexcept
    {
        return (sec > right.sec) || ((sec == right.sec) && (nsec > right.nsec));
    }

    nanoseconds delta(const utils::timespec & other) const noexcept
    {
        if (*this > other)
        {
            return chr::duration_cast<nanoseconds>(seconds(sec - other.sec)) + nanoseconds(nsec - other.nsec);
        }

        return chr::duration_cast<nanoseconds>(seconds(other.sec - sec)) + nanoseconds(other.nsec - nsec);
    }

    static constexpr utils::timespec never_expires() noexcept
    {
        return utils::timespec{};
    }

    static constexpr utils::timespec preserve_expiration() noexcept
    {
        return utils::timespec{seconds::zero(), nanoseconds(-1)};
    }

    // 64-bit may look over kill, but gives us room if in the future
    // we need to increase the resolution from ns to ps
    // in addition on 64-bit platform alignment would make the structure of similar size anyway
    seconds sec{0};
    nanoseconds nsec{0};
};

static_assert(sizeof(utils::timespec) == (sizeof(std::uint64_t) + sizeof(std::uint64_t)), "wrong size of qdb::timespec");

inline constexpr bool operator==(const utils::timespec & left, const utils::timespec & right) noexcept
{
    return (left.sec == right.sec) && (left.nsec == right.nsec);
}

inline constexpr bool operator!=(const utils::timespec & left, const utils::timespec & right) noexcept
{
    return (left.sec != right.sec) || (left.nsec != right.nsec);
}

inline constexpr bool operator<=(const utils::timespec & left, const utils::timespec & right) noexcept
{
    return !(left > right);
}

inline constexpr bool operator>=(const utils::timespec & left, const utils::timespec & right) noexcept
{
    return !(left < right);
}

template <typename Rep, typename Period>
inline timespec operator+(utils::timespec ts, chr::duration<Rep, Period> duration) noexcept
{
    ts += duration;
    return ts;
}

template <typename Rep, typename Period>
inline timespec operator-(utils::timespec ts, chr::duration<Rep, Period> duration) noexcept
{
    ts -= duration;
    return ts;
}

} 

BOOST_FUSION_ADAPT_STRUCT(utils::timespec, sec, nsec);

namespace std
{

template <typename Output>
Output & operator<<(Output & o, const ::utils::timespec & ts)
{
    o << ts.sec.count() << "." << ts.nsec.count();
    return o;
}

} // namespace std
