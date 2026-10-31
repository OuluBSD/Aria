#ifndef _Aria_CalendarScraper_h_
#define _Aria_CalendarScraper_h_

class CalendarScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;

public:
	CalendarScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
	ValueArray ScrapeEvents();
};

#endif
