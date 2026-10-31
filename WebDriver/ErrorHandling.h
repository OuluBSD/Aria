#ifndef _WebDriver_ErrorHandling_h_
#define _WebDriver_ErrorHandling_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

#define WEBDRIVERXX_CHECK(condition, message) \
	if (!(condition)) { \
		throw std::runtime_error(message); \
	}

#define WEBDRIVERXX_THROW(message) \
	throw std::runtime_error(message)

#define WEBDRIVERXX_FUNCTION_CONTEXT_BEGIN() \
	try {

#define WEBDRIVERXX_FUNCTION_CONTEXT_END_EX(context_info) \
	} catch (const std::exception& ex) { \
		throw std::runtime_error(String(ex.what()) + " - Context: " + context_info); \
	}

struct Fmt {
	String str;
	
	Fmt& operator<<(const char* s) { str += s; return *this; }
	Fmt& operator<<(const String& s) { str += s; return *this; }
	Fmt& operator<<(int n) { str += AsString(n); return *this; }
	Fmt& operator<<(double d) { str += AsString(d); return *this; }
	
	operator String() const { return str; }
};

} // namespace detail

END_UPP_NAMESPACE

#endif