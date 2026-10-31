#ifndef _WebDriver_Chrome_h_
#define _WebDriver_Chrome_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace chrome {

struct PerfLoggingPrefs : public Moveable<PerfLoggingPrefs> {
	bool enable_network = false;
	bool enable_page = false;
	bool enable_timeline = false;
	String tracing_categories;
	int buffer_usage_reporting_interval = 0;
	
	void Jsonize(JsonIO& json) {
		json("enableNetwork", enable_network)
			("enablePage", enable_page)
			("enableTimeline", enable_timeline)
			("tracingCategories", tracing_categories)
			("bufferUsageReportingInterval", buffer_usage_reporting_interval);
	}
};

} // namespace chrome

struct ChromeOptions : public Moveable<ChromeOptions> { // copyable
	Vector<String> args;
	String binary;
	Vector<String> extensions;
	Value local_state;
	Value prefs;
	bool detach = false;
	String debugger_address;
	Vector<String> exclude_switches;
	String minidump_path;
	Value mobile_emulation;
	chrome::PerfLoggingPrefs perf_logging_prefs;
	
	void Jsonize(JsonIO& json) {
		json("args", args)("extensions", extensions)
			 ("detach", detach)
			 ("debuggerAddress", debugger_address)("excludeSwitches", exclude_switches)
			 ("minidumpPath", minidump_path)("perfLoggingPrefs", perf_logging_prefs);
		
		if (json.IsStoring()) {
			if (!binary.IsEmpty()) json.Set("binary", binary);
			if (!local_state.IsVoid()) json.Set("localState", local_state);
			if (!prefs.IsVoid()) json.Set("prefs", prefs);
			if (!mobile_emulation.IsVoid()) json.Set("mobileEmulation", mobile_emulation);
		} else {
			binary = (String)json.Get("binary");
			local_state = json.Get("localState");
			prefs = json.Get("prefs");
			mobile_emulation = json.Get("mobileEmulation");
		}
	}
};

struct Chrome : Capabilities { // copyable
	ChromeOptions options;
	
	Chrome(const Capabilities& defaults = Capabilities())
		: Capabilities(defaults) {
		browser_name = browser::K_CHROME;
		version = String();
	}

	// See https://sites.google.com/a/chromium.org/chromedriver/capabilities for details
	void Jsonize(JsonIO& json) {
		Capabilities::Jsonize(json);
		json("goog:chromeOptions", options);
	}
};

END_UPP_NAMESPACE

#endif
