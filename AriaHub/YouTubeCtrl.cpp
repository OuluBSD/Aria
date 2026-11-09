#include "YouTubeCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

YouTubeCtrl::YouTubeCtrl() {
	Add(tabs.VSizePos(0, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 150));
	
	tabs.Add(feedList.SizePos(), "Feed");
	tabs.Add(studioList.SizePos(), "Studio");
	tabs.Add(commentList.SizePos(), "Comments");
	
	feedList.AddColumn("Title", 5);
	feedList.AddColumn("Author", 2);
	feedList.AddColumn("Views", 2);
	
	studioList.AddColumn("Title", 5);
	studioList.AddColumn("Views", 2);
	studioList.AddColumn("Likes", 2);
	studioList.AddColumn("Comments", 2);
	
	commentList.AddColumn("Author", 2);
	commentList.AddColumn("Content", 5);
	commentList.AddColumn("Time", 2);
	
	btnRefresh.SetLabel("Refresh YouTube");
	btnRefresh << [=] { Scrape(); };
}

void YouTubeCtrl::Scrape() {
	if (!navigator || !site_manager) return;
	
	YouTubeScraper scraper(*navigator, *site_manager);
	try {
		if (scraper.ScrapeAll()) {
			LoadData();
		} else {
			AriaAlert("YouTube sync failed. Check connection.");
		}
	} catch (const Exc& e) {
		AriaAlert("Error syncing YouTube: " + e);
	}
}

void YouTubeCtrl::LoadData() {
	LoadFeed();
	LoadStudio();
	LoadComments();
}

void YouTubeCtrl::LoadFeed() {
	YouTubeManager ym;
	ym.Load();
	feedList.Clear();
	for(int i = 0; i < ym.videos.GetCount(); i++) {
		const auto& v = ym.videos[i];
		feedList.Add(v.title, v.author, v.views);
	}
}

void YouTubeCtrl::LoadStudio() {
	if (!site_manager) return;
	studioList.Clear();
	Value data = site_manager->GetSiteData("youtube_studio", "videos");
	if (data.Is<ValueArray>()) {
		for (const Value& v : (ValueArray)data) {
			studioList.Add(v["title"], v["views"], v["likes"], v["comments"]);
		}
	}
}

void YouTubeCtrl::LoadComments() {
	YouTubeManager ym;
	ym.Load();
	commentList.Clear();
	for(int i = 0; i < ym.comments.GetCount(); i++) {
		const auto& c = ym.comments[i];
		commentList.Add(c.author, c.content, c.time);
	}
}

END_UPP_NAMESPACE