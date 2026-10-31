#include "Aria.h"

NAMESPACE_UPP

bool YouTubeScraper::ScrapeAll() {
	ym.Load();
	bool ok = true;
	ok &= ScrapeFeed();
	ok &= ScrapeStudio();
	ym.Store();
	return ok;
}

bool YouTubeScraper::ScrapeFeed() {
	Cout() << "YouTubeScraper: Navigating to YouTube Feed...\n";
	navigator.Navigate("https://www.youtube.com/");
	Sleep(10000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const videos = [];
			const items = document.querySelectorAll('ytd-rich-item-renderer');
			for(const item of items) {
				const title_el = item.querySelector('#video-title');
				const author_el = item.querySelector('#text > a');
				const meta_el = item.querySelector('#metadata-line');
				const link_el = item.querySelector('a#thumbnail');
				
				if(title_el && link_el) {
					videos.push({
						title: title_el.innerText,
						author: author_el ? author_el.innerText : "Unknown",
						views: meta_el ? meta_el.innerText : "",
						url: link_el.href
					});
				}
			}
			return videos;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "YouTubeScraper: Extracted " << va.GetCount() << " feed items.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String url = m["url"];
			YouTubeVideo& v = ym.videos.GetAdd(url);
			v.title = m["title"];
			v.author = m["author"];
			v.views = m["views"];
			v.url = url;
			int q = url.Find("?v=");
			if (q >= 0) v.id = url.Mid(q + 3);
		}
	}
	return true;
}

bool YouTubeScraper::ScrapeStudio() {
	Cout() << "YouTubeScraper: Navigating to YouTube Studio...\n";
	navigator.Navigate("https://studio.youtube.com/");
	Sleep(15000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const analytics = {};
			const dashboard = document.querySelector('ytcp-app');
			if(dashboard) {
				const snapshot = document.querySelector('ytcp-video-snapshot');
				if(snapshot) analytics.summary = snapshot.innerText;
			}
			return analytics;
		})()
	)");
	
	if(res.Is<ValueMap>()) {
		ym.analytics = res;
		Cout() << "YouTubeScraper: Updated Studio analytics.\n";
	}
	
	return true;
}

bool YouTubeScraper::ScrapeComments(const String& video_url) {
	Cout() << "YouTubeScraper: Scraping comments for " << video_url << "...\n";
	navigator.Navigate(video_url);
	Sleep(10000);
	
	navigator.Eval("window.scrollTo(0, 800)");
	Sleep(5000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const comments = [];
			const items = document.querySelectorAll('ytd-comment-thread-renderer');
			for(const item of items) {
				const author = item.querySelector('#author-text')?.innerText || "Unknown";
				const content = item.querySelector('#content-text')?.innerText || "";
				if(content) {
					comments.push({ author: author.trim(), content: content.trim() });
				}
			}
			return comments;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "YouTubeScraper: Found " << va.GetCount() << " comments.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String author = m["author"];
			String content = m["content"];
			YouTubeComment& c = ym.comments.GetAdd(author + content.Left(30));
			c.author = author;
			c.content = content;
			c.time = GetUtcTime();
		}
	}
	
	return true;
}

END_UPP_NAMESPACE