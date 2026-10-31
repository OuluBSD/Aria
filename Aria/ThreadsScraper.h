#ifndef _Aria_ThreadsScraper_h_
#define _Aria_ThreadsScraper_h_

class ThreadsScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	Time   last_refresh;
	static const char* URL;

public:
	ThreadsScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
	bool RefreshFeed();
	bool RefreshPublic();
	bool RefreshPrivate();
	
	ValueArray ScrapePage();
	ValueArray ScrapeFeed();
	ValueArray ScrapePublic();
	ValueArray ScrapePrivate();
	ValueArray ScrapeThread(const String& postUrl);
};

#endif
