#ifndef _Aria_DuunitoriScraper_h_
#define _Aria_DuunitoriScraper_h_

class AriaNavigator;
class SiteManager;

class DuunitoriScraper {
	AriaNavigator& navigator;
	SiteManager&   sm;
	String         site_name;
	String         output_dir;

	static const char* URL;

public:
	DuunitoriScraper(AriaNavigator& navigator, SiteManager& sm, const String& output_dir = "");

	bool Navigate(const String& area = "oulu");
	bool Scrape(const String& area = "oulu", const String& query = "");
};

#endif
