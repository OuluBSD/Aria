#ifndef _WebDriver_Wait_h_
#define _WebDriver_Wait_h_

#include <Core/Core.h>

NAMESPACE_UPP

template<typename T>
bool WaitUntil(
	const T& condition,
	const Session& session,
	int timeout_ms = 5000,
	int interval_ms = 500
) {
	int start = GetTickCount();
	while (int(GetTickCount()) - start < timeout_ms) {
		if (condition(session)) return true;
		Sleep(interval_ms);
	}
	return false;
}

template<typename T>
bool WaitFor(
	const T& condition,
	const Session& session,
	int timeout_ms = 5000,
	int interval_ms = 500
) {
	return WaitUntil(condition, session, timeout_ms, interval_ms);
}

END_UPP_NAMESPACE

#endif