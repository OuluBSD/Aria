#ifndef _Aria_DiscordScraper_h_
#define _Aria_DiscordScraper_h_

class DiscordScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;
	static const char* URL;
	String media_dir;

	bool WaitForStableMessages(int timeout_seconds = 90);
	void DownloadAttachments(ValueArray& messages) const;
	bool ClickResource(const String& url);

public:
	DiscordScraper(AriaNavigator& navigator, SiteManager& sm);
	
	bool Navigate();
	bool Refresh(bool deep = false);
	ValueMap ScrapeChannel(const String& url, bool no_theme = false);
	ValueMap ScrapeGuild(const String& url, bool no_theme = false, bool incremental = false);
};

#endif
