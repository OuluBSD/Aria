#ifndef _WebDriver_Conversions_h_
#define _WebDriver_Conversions_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Forward declarations for JSON conversion functions
template<typename T>
T FromJson(const Value& value) {
	T var;
	LoadFromJsonValue(var, value);
	return var;
}

template<typename T>
Value ToJson(const T& value) {
	return StoreAsJsonValue(value);
}

// Specialization for Value
template<>
inline Value FromJson<Value>(const Value& value) {
	return value;
}

template<>
inline Value ToJson<Value>(const Value& value) {
	return value;
}

// Specialization for String
template<>
inline String FromJson<String>(const Value& value) {
	return IsString(value) ? (String)value : String();
}

template<>
inline Value ToJson<String>(const String& value) {
	return Value(value);
}

// Specialization for int
template<>
inline int FromJson<int>(const Value& value) {
	return IsNumber(value) ? (int)value : 0;
}

template<>
inline Value ToJson<int>(const int& value) {
	return Value(value);
}

// Specialization for bool
template<>
inline bool FromJson<bool>(const Value& value) {
	return IsNumber(value) ? (bool)value : false;
}

template<>
inline Value ToJson<bool>(const bool& value) {
	return Value(value);
}

// Specialization for double
template<>
inline double FromJson<double>(const Value& value) {
	return IsNumber(value) ? (double)value : 0.0;
}

template<>
inline Value ToJson<double>(const double& value) {
	return Value(value);
}

// Specialization for ValueArray
template<>
inline ValueArray FromJson<ValueArray>(const Value& value) {
	return value.Is<ValueArray>() ? ValueArray(value) : ValueArray();
}

template<>
inline Value ToJson<ValueArray>(const ValueArray& value) {
	return Value(value);
}

// Specialization for ValueMap
template<>
inline ValueMap FromJson<ValueMap>(const Value& value) {
	return value.Is<ValueMap>() ? ValueMap(value) : ValueMap();
}

template<>
inline Value ToJson<ValueMap>(const ValueMap& value) {
	return Value(value);
}

END_UPP_NAMESPACE

#endif
