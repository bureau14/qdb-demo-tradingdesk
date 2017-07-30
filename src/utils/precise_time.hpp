#pragma once

#include "timespec.hpp"
#include <boost/predef/os.h>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#if !BOOST_OS_WINDOWS
#error "Unsupported OS"
#endif

namespace utils
{

using get_timespec_signature = utils::timespec (*)();
get_timespec_signature get_timespec_function();

} // namespace qdb
