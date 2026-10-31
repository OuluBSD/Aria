#ifndef _WebDriver_Ie_h_
#define _WebDriver_Ie_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct IeOptions : public Moveable<IeOptions> { // copyable
	String initial_browser_url;
	bool introduce_installs_done = false;
	bool add_document_to_collection = false;
	bool ignore_protected_mode_settings = false;
	bool ignore_zoom_setting = false;
	bool enable_persistent_hover = false;
	bool require_window_focus = false;
	bool enable_element_cache_cleanup = false;
	bool use_per_process_proxy = false;
	int browser_attach_timeout = 0;
	bool force_create_process_api = false;
	bool force_shell_windows_api = false;
	bool clean_session = false;
	
	void Jsonize(JsonIO& json) {
		json("initialBrowserUrl", initial_browser_url)
			("introduceInstallsDone", introduce_installs_done)
			("addDocumentToCollection", add_document_to_collection)
			("ignoreProtectedModeSettings", ignore_protected_mode_settings)
			("ignoreZoomSetting", ignore_zoom_setting)
			("enablePersistentHover", enable_persistent_hover)
			("requireWindowFocus", require_window_focus)
			("enableElementCacheCleanup", enable_element_cache_cleanup)
			("usePerProcessProxy", use_per_process_proxy)
			("browserAttachTimeout", browser_attach_timeout)
			("forceCreateProcessApi", force_create_process_api)
			("forceShellWindowsApi", force_shell_windows_api)
			("cleanSession", clean_session);
	}
};

struct Ie : Capabilities { // copyable
	IeOptions options;
	
	Ie(const Capabilities& defaults = Capabilities())
		: Capabilities(defaults) {
		browser_name = browser::K_INTERNET_EXPLORER;
		version = String();
	}

	void Jsonize(JsonIO& json) {
		Capabilities::Jsonize(json);
		json("se:ieOptions", options);
	}
};

END_UPP_NAMESPACE

#endif
