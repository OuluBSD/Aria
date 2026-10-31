#ifndef _Aria_ManagerSkylarkScraper_h_
#define _Aria_ManagerSkylarkScraper_h_

class AriaNavigator;
class SiteManager;

class ManagerSkylarkScraper {
	AriaNavigator& navigator;
	SiteManager&   sm;
	String         site_name;
	String         base_url;
	String         output_dir;

public:
	ManagerSkylarkScraper(AriaNavigator& navigator, SiteManager& sm,
	                      const String& base_url = "http://127.0.0.1:8080",
	                      const String& output_dir = "");

	bool Navigate(const String& subpath = "");
	ValueMap CheckStatus();
	ValueMap ScrapeOverview();
	ValueMap InspectChat();
	ValueMap SendChatMessage(const String& prompt, const String& role = "pair-programmer", int wait_secs = 10);
	bool RunSelfTests();
};

#endif
