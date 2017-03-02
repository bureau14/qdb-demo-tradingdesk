#include "precise_time.hpp"
#include "timestamp_unit.hpp"
#include "utils.hpp"

namespace utils
{
namespace detail
{
static std::uint64_t clock_state_filetime_offset() noexcept
{
    FILETIME ft;
    std::uint64_t tmpres = 0;

    // 100-nanosecond intervals since January 1, 1601 (UTC)
    // which means 0.1 us
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32ull;
    tmpres |= ft.dwLowDateTime;

    // January 1st, 1970 - January 1st, 1601 UTC ~ 369 years
    // or 116444736000000000 us
    static constexpr std::uint64_t delta_epoch = 116'444'736'000'000'000ull;
    tmpres -= delta_epoch;

    return tmpres;
}

struct windows_clock_impl
{
    // arbitrary value in update_frequency units per s
    static std::uint64_t timestamp() noexcept
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);

        // there is an imprecision with the initial value, but what matters is that timestamps are monotonic and
        // consistent
        return static_cast<std::uint64_t>(li.QuadPart);
    }

    // offset in microseconds
    static std::uint64_t zero_time() noexcept
    {
        return detail::clock_state_filetime_offset();
    }

    static std::uint64_t update_frequency() noexcept
    {
        LARGE_INTEGER li;

        // From MSDN:
        // The frequency of the performance counter is fixed at system boot and is consistent across all processors.
        // Therefore, the frequency need only be queried upon application initialization, and the result can be
        // cached.
        if (!QueryPerformanceFrequency(&li) || !li.QuadPart)
        {
            std::terminate();
        }

        return static_cast<std::uint64_t>(li.QuadPart);
    }
};

template <typename T, typename P>
struct clock_state_base
{
    // because the frequency is in update per seconds, we have to divide it by 10_M to have the correct scale
    static constexpr std::uint64_t factor = 10'000'000;

    clock_state_base() : zero_time(T::zero_time()), offset(T::timestamp())
    {
        assert(zero_time > 0u);
        assert(offset > 0u);
    }

    std::uint64_t delta() const noexcept
    {
        return static_cast<const P *>(this)->delta(T::timestamp());
    }

    // test interface
    std::uint64_t now(std::uint64_t ts) const noexcept
    {
        assert(zero_time > 0u);
        assert(offset > 0u);
        return zero_time + static_cast<const P *>(this)->delta(ts);
    }

    std::uint64_t now() const noexcept
    {
        return now(T::timestamp());
    }

    const std::uint64_t zero_time;
    const std::uint64_t offset;
};

template <typename T>
struct clock_state : clock_state_base<T, clock_state<T>>
{
    using base_type = clock_state_base<T, clock_state<T>>;

    clock_state() : frequency(T::update_frequency())
    {
    }

    std::uint64_t delta(std::uint64_t ts) const noexcept
    {
        assert(frequency > 0u);

        // we need a 128-bit precision to prevent overflows, don't just write a * b /c
        // muldiv64 will do that for us
        return muldiv64(ts - base_type::offset, base_type::factor, frequency);
    }

    const std::uint64_t frequency;
};

} // namespace detail

using hires_clock = detail::clock_state<detail::windows_clock_impl>;

utils::timespec emulate_timestamp()
{
    static hires_clock clock_state;

    auto tmpres = clock_state.now();
    return utils::timespec{hundreds_of_nanoseconds(tmpres)};
}

using GetSystemTimePreciseAsFileTimeSignature = VOID(WINAPI *)(LPFILETIME);

static utils::timespec get_timespec(GetSystemTimePreciseAsFileTimeSignature get_system_time_precise)
{
    assert(get_system_time_precise);

    FILETIME ref;
    get_system_time_precise(&ref);

    std::uint64_t tmpres = static_cast<std::uint64_t>(ref.dwHighDateTime);
    tmpres <<= 32;
    tmpres |= static_cast<std::uint64_t>(ref.dwLowDateTime);

    // FILETIME epoch = January 1st, 1601 UTC
    // Unix epoch = January 1st, 1970
    // <Unix epoch> - <FILETIME epoch> ~= 369 years
    static constexpr std::uint64_t delta_epoch = 116'444'736'000'000'000ull;
    tmpres -= delta_epoch;

    return utils::timespec{hundreds_of_nanoseconds(tmpres)};
}

get_timespec_signature get_timespec_function()
{
    HMODULE kernel32_dll = ::LoadLibrary("kernel32.dll");
    if (!kernel32_dll)
    {
        return &emulate_timestamp;
    }

    static auto get_system_time_precise_as_file_time =
        reinterpret_cast<GetSystemTimePreciseAsFileTimeSignature>(::GetProcAddress(kernel32_dll, "GetSystemTimePreciseAsFileTime"));
    if (get_system_time_precise_as_file_time)
    {
        return +static_cast<get_timespec_signature>([]() { return get_timespec(get_system_time_precise_as_file_time); });
    }

    return &emulate_timestamp;
}
}