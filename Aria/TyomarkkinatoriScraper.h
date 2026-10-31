#ifndef _Aria_TyomarkkinatoriScraper_h_
#define _Aria_TyomarkkinatoriScraper_h_

class AriaNavigator;
class SiteManager;

class TyomarkkinatoriScraper {
	AriaNavigator& navigator;
	SiteManager&   sm;
	String         site_name;
	String         output_dir;

	static const char* URL;

public:
	TyomarkkinatoriScraper(AriaNavigator& navigator, SiteManager& sm, const String& output_dir = "");

	bool Navigate();
	bool Scrape(const String& municipality_code = "564", const String& query = "");
};

#endif
