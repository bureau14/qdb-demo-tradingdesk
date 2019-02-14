#pragma once

#include "timespec.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace utils
{
namespace detail
{

static const boost::gregorian::date epoch_date{1970, 1, 1};
static const boost::posix_time::ptime epoch_ptime{epoch_date};

inline chr::seconds to_seconds_from_epoch(boost::posix_time::ptime pt)
{
    auto total_seconds = (pt - epoch_ptime).total_seconds();
    return chr::seconds{total_seconds};
}

} // namespace detail

static inline boost::gregorian::date extract_date(const timespec & ts) noexcept
{
    return (detail::epoch_ptime + boost::posix_time::seconds{ts.sec.count()}).date();
}

static inline seconds extract_seconds(const boost::gregorian::date & d) noexcept
{
    assert(!d.is_special());
    return detail::to_seconds_from_epoch(boost::posix_time::ptime{d});
}

static inline std::tuple<boost::gregorian::date, chr::seconds, chr::nanoseconds> split_date_time(const timespec & ts) noexcept
{
    // TODO(edouard): We can probably optimize this by extracting date and seconds in one pass.
    // TODO(marek): We can optimize this by extracting date as ptime, because we use in extract_seconds() we convert again date to ptime.
    auto d = extract_date(ts);
    return std::make_tuple(d, ts.sec - extract_seconds(d), ts.nsec);
}

static inline timespec make_timespec(const boost::gregorian::date & d) noexcept
{
    return timespec{extract_seconds(d)};
}

static inline timespec make_timespec(
    boost::gregorian::date::year_type y, boost::gregorian::date::month_type m, boost::gregorian::date::day_type d) noexcept
{
    return timespec{extract_seconds(boost::gregorian::date{y, m, d})};
}

} // namespace utils
