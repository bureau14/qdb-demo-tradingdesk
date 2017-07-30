#include "time.hpp"
#include "utils.hpp"

#include <boost/predef/os.h>
#include <chrono>

#if BOOST_OS_MACOS
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include <mach/mach_types.h>
#include <mach/vm_statistics.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#endif

#if BOOST_OS_WINDOWS
#include "precise_time.hpp"
#else
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cerrno>
#include <cxxabi.h>
#include <execinfo.h>
#include <pthread.h>
#include <pwd.h>
#include <unistd.h>
#endif

namespace utils
{

#if BOOST_OS_MACOS
struct mac_hires_clock
{
    mac_hires_clock() noexcept
    {
        mach_timebase_info(&_mach_info);

        struct timeval now;
        gettimeofday(&now, NULL);

        _zero_time.sec = std::chrono::seconds(now.tv_sec);
        _zero_time.nsec = std::chrono::microseconds(now.tv_usec);

        _delta = ns_since_bootup();
    }

    chr::nanoseconds ns_since_bootup() noexcept
    {
        return std::chrono::nanoseconds{muldiv64(static_cast<std::uint64_t>(mach_absolute_time()),
            static_cast<std::uint64_t>(_mach_info.numer), static_cast<std::uint64_t>(_mach_info.denom))};
    }

    utils::timespec now() noexcept
    {
        const auto ts = ns_since_bootup() - _delta;
        assert(ts > std::chrono::nanoseconds::zero());

        return _zero_time + ts;
    }

private:
    mach_timebase_info_data_t _mach_info;
    utils::timespec _zero_time;
    std::chrono::nanoseconds _delta;
};
#endif

utils::timespec timestamp() noexcept
{
#if BOOST_OS_WINDOWS
    static auto get_timespec = get_timespec_function();

    const utils::timespec ref = get_timespec();

    utils::timespec draw = ref;

    for (; draw <= ref; draw = get_timespec())
    {
    }

    return draw;
#elif BOOST_OS_MACOS
    static mac_hires_clock clock_state;

    const utils::timespec ref = clock_state.now();

    utils::timespec draw = ref;

    for (; draw <= ref; draw = clock_state.now())
    {
    }

    return draw;
#else

#if BOOST_OS_BSD_FREE
    static constexpr clockid_t clock_type = CLOCK_REALTIME_PRECISE;
#else
    static constexpr clockid_t clock_type = CLOCK_REALTIME;
#endif

    ::timespec ref;

    clock_gettime(clock_type, &ref);

    ::timespec draw = ref;

    // draw a value that is greater than the previous one
    for (; (draw.tv_sec <= ref.tv_sec) && (draw.tv_nsec <= ref.tv_nsec); clock_gettime(clock_type, &draw))
    {
    }

    return utils::timespec{draw};
#endif
}
}
