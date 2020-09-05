#pragma once

#include <array>
#include <type_traits>

namespace utils
{

template <typename D = void, typename... T>
constexpr auto make_array(T &&... t) -> std::array<std::conditional_t<std::is_void_v<D>, std::common_type_t<T...>, D>, sizeof...(T)>
{
    return {{std::forward<T>(t)...}};
}

} // namespace utils
