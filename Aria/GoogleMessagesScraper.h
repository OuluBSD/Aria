#ifndef _Aria_GoogleMessagesScraper_h_
#define _Aria_GoogleMessagesScraper_h_

class GoogleMessagesScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;

public:
	GoogleMessagesScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
	ValueArray ScrapeAllConversations();
	ValueArray ExtractVisibleMessages();
};

#endif
