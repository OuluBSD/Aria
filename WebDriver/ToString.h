#ifndef _WebDriver_ToString_h_
#define _WebDriver_ToString_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

template<typename T>
String ToString(const T& value) {
	return AsString(value);
}

// Specializations for common types
template<>
inline String ToString<bool>(const bool& value) {
	return value ? "true" : "false";
}

template<>
inline String ToString<int>(const int& value) {
	return AsString(value);
}

template<>
inline String ToString<double>(const double& value) {
	return AsString(value);
}

template<>
inline String ToString<String>(const String& value) {
	return value;
}

} // namespace detail

END_UPP_NAMESPACE

#endif