#pragma once

#include <chrono>
#include <cstdint>

namespace utils
{

using hundreds_of_nanoseconds = std::chrono::duration<std::uint64_t, std::ratio_multiply<std::ratio<100>, std::nano>>;

} // namespace qdb
