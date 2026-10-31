#ifndef _Aria_SlackScraper_h_
#define _Aria_SlackScraper_h_

class SlackScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	String site_name;

	bool SelectSlackTab(int expected_tabs);
	bool ClickChannel(const String& channel);
	void LoadOlderMessages();
	bool ClickThreadReplies(const String& message_id);
	ValueMap ExtractThread(const String& channel_id, const String& parent_message_id);
	ValueMap ExtractChannel(const String& requested);

public:
	SlackScraper(AriaNavigator& navigator, SiteManager& sm);

	// Scrape rendered messages from an existing, authenticated Slack tab.
	// No Slack navigation, API calls, writes, or new tabs are performed.
	ValueMap Scrape(const Vector<String>& channels = Vector<String>());
	bool Refresh(const Vector<String>& channels = Vector<String>());
};

#endif
