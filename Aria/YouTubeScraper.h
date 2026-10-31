#ifndef _Aria_YouTubeScraper_h_
#define _Aria_YouTubeScraper_h_

class YouTubeScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	YouTubeManager ym;
	bool force = false;

public:
	YouTubeScraper(AriaNavigator& nav, SiteManager& sm) : navigator(nav), sm(sm) {}

	void SetForce(bool b = true) { force = b; }

	bool ScrapeAll();
	bool ScrapeFeed();
	bool ScrapeStudio();
	bool ScrapeComments(const String& video_url);

	const YouTubeManager& GetManager() const { return ym; }
	YouTubeManager& GetManager() { return ym; }
	
	void Load() { ym.Load(); }
	void Store() { ym.Store(); }
};

#endif