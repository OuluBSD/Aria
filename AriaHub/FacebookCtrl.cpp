#include "FacebookCtrl.h"

NAMESPACE_UPP

FacebookCtrl::FacebookCtrl() {
	Add(fb_tabs.SizePos());
	
	feed_list.AddColumn("Time");
	feed_list.AddColumn("Author");
	feed_list.AddColumn("Content", 4);
	
	friends_list.AddColumn("Name");
	friends_list.AddColumn("URL");
	
	fb_tabs.Add(feed_list.SizePos(), "Feed");
	fb_tabs.Add(friends_list.SizePos(), "Friends");
	
	feed_list.LayoutId("FbFeedList");
	friends_list.LayoutId("FbFriendsList");
}

void FacebookCtrl::LoadData() {
	FacebookManager fm;
	fm.Load();
	
	feed_list.Clear();
	for(int i = 0; i < fm.feed.GetCount(); i++) {
		const auto& p = fm.feed[i];
		feed_list.Add(p.time, p.author, p.content);
	}
	
	friends_list.Clear();
	for(int i = 0; i < fm.friends.GetCount(); i++) {
		const auto& f = fm.friends[i];
		friends_list.Add(f.name, f.profile_url);
	}
}

void FacebookCtrl::Scrape() {
	if(!navigator || !site_manager) return;
	
	AriaNavigator* nav = navigator;
	SiteManager* sm = site_manager;
	
	Thread().Run([=] {
		FacebookScraper scraper(*nav, *sm);
		scraper.ScrapeAll();
		PostCallback(THISBACK(LoadData));
	});
}

END_UPP_NAMESPACE
