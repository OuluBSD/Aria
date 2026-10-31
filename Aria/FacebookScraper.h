#ifndef _Aria_FacebookScraper_h_
#define _Aria_FacebookScraper_h_

class FacebookScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	FacebookManager fm;
	bool force = false;

public:
	FacebookScraper(AriaNavigator& nav, SiteManager& sm) : navigator(nav), sm(sm) {}

	void SetForce(bool b = true) { force = b; }

	bool ScrapeAll();
	bool ScrapeFeed();
	bool ScrapeOwnPage();
	bool ScrapeFriendsList();
	bool ScrapeFriendFeed(const String& profile_url);

	const FacebookManager& GetManager() const { return fm; }
	FacebookManager& GetManager() { return fm; }
	
	void Load() { fm.Load(); }
	void Store() { fm.Store(); }
};

#endif