#ifndef _WebDriver_WaitMatch_h_
#define _WebDriver_WaitMatch_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace wait {

struct ElementPresent {
	By by;
	ElementPresent(const By& by) : by(by) {}
	bool operator()(const Session& s) const {
		try {
			s.FindElement(by);
			return true;
		} catch (...) {
			return false;
		}
	}
};

struct ElementVisible {
	By by;
	ElementVisible(const By& by) : by(by) {}
	bool operator()(const Session& s) const {
		try {
			Element e = s.FindElement(by);
			return e.IsDisplayed();
		} catch (...) {
			return false;
		}
	}
};

struct UrlContains {
	String text;
	UrlContains(const String& text) : text(text) {}
	bool operator()(const Session& s) const {
		return s.GetUrl().Find(text) >= 0;
	}
};

struct TitleIs {
	String title;
	TitleIs(const String& title) : title(title) {}
	bool operator()(const Session& s) const {
		return s.GetTitle() == title;
	}
};

} // namespace wait

template<typename T>
bool WaitUntilMatches(
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

END_UPP_NAMESPACE

#endif
