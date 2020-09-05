#pragma once

#include <array>
#include <string>

namespace utils
{

std::string humanize_number(std::uint64_t n, const char * const unit = "B", double thousands = 1024.0);

} // namespace utils
