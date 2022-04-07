#ifndef UTIL_HPP
#define UTIL_HPP

#include <type_traits>

namespace boot {

template <typename T>
std::remove_reference_t<T>&& Move(T&& in)
{
    return static_cast<std::remove_reference_t<T>&&>(in);
}

} // namespace boot

#endif // UTIL_HPP
