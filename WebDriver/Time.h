#ifndef _WebDriver_Time_h_
#define _WebDriver_Time_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

using Time_point = std::chrono::steady_clock::time_point;
using Duration = std::chrono::milliseconds;

inline Time_point Now() {
	return std::chrono::steady_clock::now();
}

template<typename Rep, typename Period>
int To_milliseconds(const std::chrono::duration<Rep, Period>& duration) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

inline Duration Milliseconds(int ms) {
	return std::chrono::milliseconds(ms);
}

template<typename Rep, typename Period>
bool Is_elapsed(const Time_point& start, const std::chrono::duration<Rep, Period>& timeout) {
	return std::chrono::steady_clock::now() - start >= timeout;
}

} // namespace detail

END_UPP_NAMESPACE

#endif