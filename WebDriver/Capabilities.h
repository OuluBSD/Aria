#ifndef _WebDriver_Capabilities_h_
#define _WebDriver_Capabilities_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace browser {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_ANDROID = "android";
ConstValue K_CHROME = "chrome";
ConstValue K_FIREFOX = "firefox";
ConstValue K_HTML_UNIT = "htmlunit";
ConstValue K_INTERNET_EXPLORER = "internet explorer";
ConstValue K_IPHONE = "iPhone";
ConstValue K_IPAD = "iPad";
ConstValue K_MOCK = "mock";
ConstValue K_OPERA = "opera";
ConstValue K_SAFARI = "safari";
ConstValue K_PHANTOM = "phantomjs";
} // namespace browser

namespace platform {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_ANY = "ANY";
ConstValue K_WINDOWS = "WINDOWS";
ConstValue K_XP = "XP";
ConstValue K_VISTA = "VISTA";
ConstValue K_MAC = "MAC";
ConstValue K_LINUX = "LINUX";
ConstValue K_UNIX = "UNIX";
ConstValue K_ANDROID = "ANDROID";
} // namespace platform

namespace unexpected_alert_behaviour {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_ACCEPT = "accept";
ConstValue K_DISMISS = "dismiss";
ConstValue K_IGNORE = "ignore";
} // namespace unexpected_alert_behaviour

namespace proxy_type {
typedef String Value;
typedef const char* const ConstValue;
ConstValue K_DIRECT = "direct";
ConstValue K_MANUAL = "manual";
ConstValue K_PAC = "pac";
ConstValue K_AUTODETECT = "autodetect";
ConstValue K_SYSTEM = "system";
} // namespace proxy_type

struct WdProxy : public Moveable<WdProxy> { // copyable
	String proxy_type;
	
	void Jsonize(JsonIO& json) {
		json("proxyType", proxy_type);
	}
};

struct LoggingPrefs : public Moveable<LoggingPrefs> {
	String level;
	
	void Jsonize(JsonIO& json) {
		json("driver", level);
	}
};

// List of keys and values indicating features that server can or should provide.
struct Capabilities : public Moveable<Capabilities> { // copyable
	String browser_name;
	String version;
	String platform;
	bool accept_insecure_certs = false;
	WdProxy proxy;
	String unhandled_prompt_behavior;
	
	Capabilities() {}
	
	void Jsonize(JsonIO& json) {
		json("browserName", browser_name);
		if (!version.IsEmpty() || !json.IsStoring())
			json("browserVersion", version);
		if ((!platform.IsEmpty() && platform != "ANY") || !json.IsStoring())
			json("platformName", platform);
		if (accept_insecure_certs || !json.IsStoring())
			json("acceptInsecureCerts", accept_insecure_certs);
		if (!proxy.proxy_type.IsEmpty() || !json.IsStoring())
			json("proxy", proxy);
		if (!unhandled_prompt_behavior.IsEmpty() || !json.IsStoring())
			json("unhandledPromptBehavior", unhandled_prompt_behavior);
	}
};

END_UPP_NAMESPACE

#endif
