#include "time.hpp"

#include <boost/predef/os.h>

#if BOOST_OS_WINDOWS
#include "precise_time.hpp"
#else // BOOST_OS_LINUX || BOOST_OS_BSD_FREE || BOOST_OS_MACOS
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