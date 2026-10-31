#ifndef _WebDriver_MetaTools_h_
#define _WebDriver_MetaTools_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

// Type trait to check if T is iterable (has begin/end methods)
template<typename T, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<
    decltype(std::begin(std::declval<T>())),
    decltype(std::end(std::declval<T>()))
>> : std::true_type {};

// Helper for conditional type selection
template<bool Condition, typename TrueType, typename FalseType>
struct if_ {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct if_<false, TrueType, FalseType> {
    using type = FalseType;
};

// Helper for type identity
template<typename T>
struct type_is {
    using type = T;
};

} // namespace detail

END_UPP_NAMESPACE

#endif