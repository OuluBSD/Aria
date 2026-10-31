#ifndef _Aria_YouTubeStudioScraper_h_
#define _Aria_YouTubeStudioScraper_h_

class YouTubeStudioScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;

public:
	YouTubeStudioScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
	ValueMap ScrapeDashboard();
	ValueArray ScrapeVideos();
};

#endif
