#ifndef _Aria_WhatsAppScraper_h_
#define _Aria_WhatsAppScraper_h_

class WhatsAppScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;

public:
	WhatsAppScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
};

#endif
