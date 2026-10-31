#include "Aria.h"

NAMESPACE_UPP

bool FacebookScraper::ScrapeAll() {
	fm.Load();
	bool ok = true;
	ok &= ScrapeFeed();
	ok &= ScrapeOwnPage();
	ok &= ScrapeFriendsList();
	fm.Store();
	return ok;
}

bool FacebookScraper::ScrapeFeed() {
	Cout() << "FacebookScraper: Navigating to Feed...\n";
	navigator.Navigate("https://www.facebook.com/");
	Sleep(10000);
	
	// Check for login wall
	Value is_logged_in = navigator.Eval("return !!document.querySelector('div[role=\"navigation\"] div[aria-label*=\"Account\"]')");
	if (!is_logged_in.To<bool>()) {
		Cout() << "FacebookScraper: Not logged in. Attempting login...\n";
		CredentialManager cm;
		String email = cm.GetCredential("facebook:email");
		String pass = cm.GetCredential("facebook:password");
		if (email.IsEmpty() || pass.IsEmpty()) {
			Cout() << "Error: Facebook credentials not found in vault.\n";
			return false;
		}
		
		navigator.Eval("document.getElementById('email').value = " + AsJSON(email));
		navigator.Eval("document.getElementById('pass').value = " + AsJSON(pass));
		navigator.Eval("document.querySelector('button[name=\"login\"]').click()");
		Sleep(15000);
	}

	Value res = navigator.Eval(R"(
		return (function() {
			const posts = [];
			const articles = document.querySelectorAll('div[role="feed"] div[role="article"], div[data-ad-preview="message"]');
			for(const art of articles) {
				const author_el = art.querySelector('h3') || art.querySelector('strong');
				const content_el = art.querySelector('div[data-ad-comet-preview="message"]');
				if(author_el && content_el) {
					posts.push({
						author: author_el.innerText,
						content: content_el.innerText
					});
				}
			}
			return posts;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "FacebookScraper: Extracted " << va.GetCount() << " feed items.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String author = m["author"];
			String content = m["content"];
			FacebookPost& p = fm.feed.GetAdd(author + content.Left(30));
			p.author = author;
			p.content = content;
			p.time = GetUtcTime();
		}
	}
	return true;
}

bool FacebookScraper::ScrapeOwnPage() {
	Cout() << "FacebookScraper: Navigating to Profile...\n";
	navigator.Navigate("https://www.facebook.com/me");
	Sleep(10000);
	return true;
}

bool FacebookScraper::ScrapeFriendsList() {
	Cout() << "FacebookScraper: Navigating to Friends List...\n";
	navigator.Navigate("https://www.facebook.com/me/friends");
	Sleep(10000);
	
	// Scroll to load more
	navigator.Eval("window.scrollTo(0, document.body.scrollHeight)");
	Sleep(3000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const friends = [];
			// Selector for friend cards in modern FB
			const cards = document.querySelectorAll('div[data-visualcompletion="ignore-dynamic"] a[role="link"]');
			for(const a of cards) {
				const name = a.innerText;
				const url = a.href;
				if (name && url && url.includes('/profile.php') || url.includes('facebook.com/')) {
					if (name.length > 1 && !friends.some(f => f.url === url)) {
						friends.push({ name: name, url: url });
					}
				}
			}
			return friends;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "FacebookScraper: Extracted " << va.GetCount() << " potential friends.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String name = m["name"];
			String url = m["url"];
			if (name.IsEmpty() || url.IsEmpty() || name == "Friends") continue;
			FacebookFriend& f = fm.friends.GetAdd(url);
			f.name = name;
			f.profile_url = url;
		}
	}
	return true;
}

bool FacebookScraper::ScrapeFriendFeed(const String& profile_url) {
	Cout() << "FacebookScraper: Scraping feed for " << profile_url << "...\n";
	navigator.Navigate(profile_url);
	Sleep(10000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const posts = [];
			const articles = document.querySelectorAll('div[role="main"] div[role="article"]');
			for(const art of articles) {
				const content_el = art.querySelector('div[data-ad-comet-preview="message"]');
				if(content_el) {
					posts.push({ content: content_el.innerText });
				}
			}
			return posts;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "Friend Feed: Found " << va.GetCount() << " posts.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			Cout() << "--- POST ---\n" << m["content"] << "\n";
		}
	}
	return true;
}

END_UPP_NAMESPACE