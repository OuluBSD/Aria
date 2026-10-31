#include "Aria.h"

NAMESPACE_UPP

const char* YouTubeStudioScraper::URL = "https://studio.youtube.com/";

YouTubeStudioScraper::YouTubeStudioScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "youtube_studio";
}

bool YouTubeStudioScraper::Navigate() {
	GetAriaLogger("youtube_studio").Info("Navigating to YouTube Studio...");
	try {
		navigator.Navigate(URL);
		navigator.WaitForElement("ytcp-app", "tag name", 45);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("youtube_studio").Error("Failed to load YouTube Studio: " + e);
		return false;
	}
}

bool YouTubeStudioScraper::Refresh(bool deep) {
	if (!Navigate()) return false;
	GetAriaLogger("youtube_studio").Info("Refreshing YouTube Studio data...");
	
	ValueMap analytics = ScrapeDashboard();
	ValueArray videos = ScrapeVideos();
	
	sm.SetSiteData(site_name, "analytics", analytics);
	sm.SetSiteData(site_name, "videos", videos);
	sm.SetSiteData(site_name, "metadata", ValueMap()
		("last_refresh", GetUtcTime())
		("video_count", videos.GetCount())
	);
	
	return true;
}

ValueMap YouTubeStudioScraper::ScrapeDashboard() {
	try {
		GetAriaLogger("youtube_studio").Info("Scraping dashboard...");
		// Use JS to extract summary text from snapshot cards
		Value res = navigator.Eval(R"(
			const cards = document.querySelectorAll('ytcp-video-snapshot, ytcp-channel-snapshot');
			let summary = "";
			cards.forEach(c => { summary += c.innerText + "\n"; });
			return { channel_summary: summary.trim() };
		)");
		
		if (res.Is<ValueMap>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("youtube_studio").Error("Error scraping dashboard: " + e);
	}
	return ValueMap();
}

ValueArray YouTubeStudioScraper::ScrapeVideos() {
	try {
		GetAriaLogger("youtube_studio").Info("Scraping videos...");
		
		// Navigate to Content page if needed
		if (navigator.GetPageContent().Find("video-list") < 0) {
			navigator.Navigate("https://studio.youtube.com/channel/videos/upload");
			Sleep(3000);
		}

		// Use complex JS to extract row data from the content table
		Value res = navigator.Eval(R"(
			const videos = [];
			const rows = document.querySelectorAll('ytcp-video-row');
			rows.forEach(row => {
				try {
					const titleEl = row.querySelector('#video-title');
					const cells = row.querySelectorAll('ytcp-video-cell-text');
					
					if (titleEl) {
						const video = {
							title: titleEl.innerText.trim(),
							views: "0",
							likes: "0",
							comments: "0",
							ctr: "0%"
						};
						
						// Heuristic mapping of cells
						// 0: Visibility, 1: Restrictions, 2: Date, 3: Views, 4: Comments, 5: Likes/Dislikes
						if (cells.length >= 6) {
							video.views = cells[3].innerText.trim();
							video.comments = cells[4].innerText.trim();
							video.likes = cells[5].innerText.trim();
						}
						
						videos.push(video);
					}
				} catch(e) {}
			});
			return videos;
		)");
		
		if (res.Is<ValueArray>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("youtube_studio").Error("Error scraping videos: " + e);
	}
	return ValueArray();
}

END_UPP_NAMESPACE
