#include "Aria.h"

NAMESPACE_UPP

const char* ThreadsScraper::URL = "https://www.threads.net/";

ThreadsScraper::ThreadsScraper(AriaNavigator& navigator, SiteManager& sm)
	: navigator(navigator)
	, sm(sm)
{
	site_name = "threads";
}

bool ThreadsScraper::Navigate() {
	try {
		String current_url = navigator.Eval("return window.location.href");
		if (current_url.Find("threads.net") >= 0) {
			GetAriaLogger("threads").Info("Already on Threads page.");
			return true;
		}
	} catch(...) {}

	GetAriaLogger("threads").Info("Navigating to Threads...");
	try {
		navigator.Navigate(URL);
		
		// Force visibility and focus events
		navigator.Eval(R"(
			document.dispatchEvent(new Event('visibilitychange'));
			window.dispatchEvent(new Event('focus'));
		)");

		// Wait for either the feed or the login screen
		navigator.WaitForElement("svg[aria-label='Threads'], div[aria-label='Log in'], a[href='/login/']", "css selector", 45);
		
		// Wait an extra 5 seconds for background JS to fetch more items as requested
		GetAriaLogger("threads").Info("Waiting 5s for dynamic content fetch...");
		Sleep(5000);

		// Check for login indicators
		Value login_check = navigator.Eval(R"(
			return (function() {
				if (document.querySelector('div[aria-label="Log in"]') || document.querySelector('a[href="/login/"]')) return "login_screen";
				if (document.querySelector('svg[aria-label="Threads"]')) return "logged_in";
				return "unknown";
			})()
		)");
		
		if (login_check == "login_screen") {
			GetAriaLogger("threads").Error("Not logged in to Threads. Please log in manually with the profile first.");
			return false;
		}
		
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Failed to load Threads: " + e);
		return false;
	}
}

bool ThreadsScraper::Refresh(bool deep) {
	Time now = GetUtcTime();
	if (now - last_refresh < 60) {
		GetAriaLogger("threads").Info("Skipping refresh (cooldown active, last refresh < 1m ago).");
		return true;
	}

	if (!Navigate()) return false;
	GetAriaLogger("threads").Info(Format("Starting %s data refresh for Threads...", deep ? "deep" : "shallow"));
	
	ValueArray feed = ScrapeFeed();
	GetAriaLogger("threads").Info(Format("Found %d posts in general feed.", feed.GetCount()));
	
	sm.SetSiteData(site_name, "feed", feed);
	sm.SetSiteData(site_name, "metadata", ValueMap()
		("last_refresh", now)
		("mode", deep ? "deep" : "shallow")
	);
	
	last_refresh = now;
	return true;
}

bool ThreadsScraper::RefreshFeed() {
	Time now = GetUtcTime();
	if (now - last_refresh < 60) {
		GetAriaLogger("threads").Info("Skipping feed refresh (cooldown active).");
		return true;
	}

	if (!Navigate()) return false;
	ValueArray feed = ScrapeFeed();
	sm.SetSiteData(site_name, "feed", feed);
	last_refresh = now;
	return true;
}

bool ThreadsScraper::RefreshPublic() {
	if (!Navigate()) return false;
	GetAriaLogger("threads").Info("Navigating to user profile...");
	try {
		// Click profile link/icon
		navigator.FindElement(By::CssSelector("a[href^='/@']")).Click();
		Sleep(2000);
		ValueArray posts = ScrapePublic();
		sm.SetSiteData(site_name, "public", posts);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Failed to refresh public messages: " + e);
		return false;
	}
}

bool ThreadsScraper::RefreshPrivate() {
	if (!Navigate()) return false;
	GetAriaLogger("threads").Info("Navigating to activity/messages...");
	try {
		// Navigate to activity as a proxy for private interactions
		navigator.Navigate("https://www.threads.net/notifications");
		Sleep(2000);
		ValueArray items = ScrapePrivate();
		sm.SetSiteData(site_name, "private", items);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Failed to refresh private messages: " + e);
		return false;
	}
}

ValueArray ThreadsScraper::ScrapePage() {
	try {
		// Use JS to extract posts
		Value res = navigator.Eval(R"(
			return (function() {
				const posts = [];
				const containers = document.querySelectorAll('article, [role="article\"], [data-pressable-container="true"]');
				for (let i = 0; i < containers.length; i++) {
					const art = containers[i];
					try {
						// 1. Find author
						const links = art.querySelectorAll('a');
						let author = "";
						for (let j = 0; j < links.length; j++) {
							const href = links[j].getAttribute('href') || "";
							if (href.includes('@')) {
								author = links[j].innerText.trim();
								if (author) break;
							}
						}
						if (!author) continue;

						// 2. Find content
						let content = "";
						const candidates = art.querySelectorAll('[dir="auto"]');
						for (let k = 0; k < candidates.length; k++) {
							const node = candidates[k];
							if (node.querySelector('[dir="auto"]')) continue;
							
							const txt = (node.innerText || node.textContent || "").trim();
							if (!txt || txt === author || author.includes(txt)) continue;
							if (txt.match(/^\d+\s*[hdmsw]$/) || txt === "•" || txt === "Reply" || txt === "Threads") continue;
							
							if (txt.length > content.length) content = txt;
						}

						// 3. Find ID
						let id = "";
						for (let l = 0; l < links.length; l++) {
							const h = links[l].getAttribute('href') || "";
							if (h.includes('/post/')) {
								id = h;
								break;
							}
						}

						if (author && content && content.length > 2) {
							if (!posts.some(p => p.author === author && p.content === content)) {
								posts.push({
									id: id || (author + "_" + content.substring(0, 20)),
									author: author,
									content: content,
									likes: 0,
									replies: 0,
									reposts: 0,
									timestamp: Date.now()
								});
							}
						}
					} catch(e) {}
				}
				return { count: containers.length, posts: posts };
			})()
		)");
		
		if (res.Is<ValueMap>()) {
			ValueArray posts = res["posts"];
			GetAriaLogger("threads").Info(Format("Scraper saw %d containers, extracted %d posts.", (int)res["count"], posts.GetCount()));
			return posts;
		}
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error in ScrapePage: " + e);
	}
	return ValueArray();
}

ValueArray ThreadsScraper::ScrapeFeed() {
	try {
		GetAriaLogger("threads").Info("Scraping feed...");
		
		// Wait for at least one post to appear
		bool found = false;
		for (int i = 0; i < 10; i++) {
			Value count = navigator.Eval("return document.querySelectorAll('article, [role=\"article\"], [data-pressable-container=\"true\"]').length;");
			if (count.Is<double>() && (double)count > 0) {
				found = true;
				break;
			}
			GetAriaLogger("threads").Info("Waiting for posts to load...");
			Sleep(1000);
		}
		
		if (!found) {
			GetAriaLogger("threads").Warning("No posts appeared after waiting.");
		}

		ValueArray new_posts = ScrapePage();
		
		// Load existing data to merge
		Value existing = sm.GetSiteData(site_name, "feed");
		if (existing.Is<ValueArray>()) {
			VectorMap<String, Value> merged;
			// Load old
			for(const Value& p : (ValueArray)existing) {
				String id = p["id"];
				if(id.IsEmpty()) id = String(p["author"]) + "_" + String(p["content"]).Left(20);
				merged.GetAdd(id) = p;
			}
			// Merge new
			for(const Value& p : new_posts) {
				String id = p["id"];
				if(id.IsEmpty()) id = String(p["author"]) + "_" + String(p["content"]).Left(20);
				merged.GetAdd(id) = p;
			}
			
			ValueArray result;
			for(const Value& v : merged.GetValues()) result.Add(v);
			return result;
		}
		
		return new_posts;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error scraping feed: " + e);
	}
	return ValueArray();
}

ValueArray ThreadsScraper::ScrapeThread(const String& postUrl) {
	try {
		String fullUrl = postUrl;
		if (!fullUrl.StartsWith("http")) {
			fullUrl = String("https://www.threads.net") + (postUrl.StartsWith("/") ? "" : "/") + postUrl;
		}
		
		GetAriaLogger("threads").Info("Navigating to thread: " + fullUrl);
		navigator.Navigate(fullUrl);
		
		// Force visibility
		navigator.Eval(R"(
			document.dispatchEvent(new Event('visibilitychange'));
			window.dispatchEvent(new Event('focus'));
		)");

		// Wait for articles/posts to load
		navigator.WaitForElement("article, [role='article']", "css selector", 30);
		Sleep(3000); // Wait for replies to populate

		return ScrapePage(); // Returns only what's on this specific thread page
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error scraping thread: " + e);
	}
	return ValueArray();
}

ValueArray ThreadsScraper::ScrapePublic() {
	GetAriaLogger("threads").Info("Scraping public profile...");
	return ScrapeFeed(); // Reuse feed scraping logic for profile posts
}

ValueArray ThreadsScraper::ScrapePrivate() {
	GetAriaLogger("threads").Info("Scraping activity/notifications...");
	try {
		Value res = navigator.Eval(R"(
			const items = [];
			const els = document.querySelectorAll('div[role="listitem"]');
			els.forEach(el => {
				try {
					const textEl = el.innerText;
					if (textEl) {
						items.push({
							author: "System",
							content: textEl.trim()
						});
					}
				} catch(e) {}
			});
			return items;
		)");
		if (res.Is<ValueArray>()) return res;
	} catch (const Exc& e) {
		GetAriaLogger("threads").Error("Error scraping activity: " + e);
	}
	return ValueArray();
}

END_UPP_NAMESPACE