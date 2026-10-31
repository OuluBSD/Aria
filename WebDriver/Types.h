#ifndef _WebDriver_Types_h_
#define _WebDriver_Types_h_

#include <Core/Core.h>


NAMESPACE_UPP

typedef unsigned long long TimePoint;
typedef unsigned Duration;

typedef Point Offset;

struct Cookie : public Moveable<Cookie> {
	enum {
		NO_EXPIRY = 0
	};

	String name;
	String value;
	String path;
	String domain;
	bool secure;
	bool http_only;
	int expiry; // seconds since midnight, January 1, 1970 UTC

	Cookie() : secure(false), http_only(false), expiry(NO_EXPIRY) {}
	Cookie(
		const String& name,
		const String& value,
		const String& path = String(),
		const String& domain = String(),
		bool secure = false,
		bool http_only = false,
		int expiry = NO_EXPIRY
		)
		: name(name)
		, value(value)
		, path(path)
		, domain(domain)
		, secure(secure)
		, http_only(http_only)
		, expiry(expiry)
	{}

	bool operator == (const Cookie& c) const {
		return name == c.name
			&& value == c.value
			&& path == c.path
			&& domain == c.domain
			&& secure == c.secure
			&& http_only == c.http_only
			&& expiry == c.expiry
			;
	}

	void Jsonize(JsonIO& json) {
		json("name", name)("value", value)("path", path)("domain", domain)
			("secure", secure)("httpOnly", http_only)("expiry", expiry);
	}
};

namespace timeout {

typedef const char* Type;

Type const Implicit = "implicit";
Type const PageLoad = "page load";
Type const Script = "script";

} // namespace timeout

namespace mouse {
enum Button {
	LEFT_BUTTON = 0,
	MIDDLE_BUTTON = 1,
	RIGHT_BUTTON = 2
};
} // namespace mouse

inline void Jsonize(JsonIO& jio, Size& s) {
	double width = s.cx, height = s.cy;
	jio("width", width)("height", height);
	s.cx = (int)width; s.cy = (int)height;
}

inline void Jsonize(JsonIO& jio, Point& p) {
	double x = p.x, y = p.y;
	jio("x", x)("y", y);
	p.x = (int)x; p.y = (int)y;
}

END_UPP_NAMESPACE

#endif