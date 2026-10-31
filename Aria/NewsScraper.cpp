#include "NewsScraper.h"

NAMESPACE_UPP

NewsScraper::NewsScraper(AriaNavigator& navigator, SiteManager& sm)
: navigator(navigator), sm(sm)
{}

ValueArray NewsScraper::ScrapeSite(const String& name, const String& url) {
	GetAriaLogger("news").Info("Navigating to " + name + " (" + url + ")...");
	try {
		navigator.Navigate(url);
		
		// Force visibility
		navigator.Eval(R"(
			document.dispatchEvent(new Event('visibilitychange'));
			window.dispatchEvent(new Event('focus'));
		)");

		Sleep(5000); // 5s wait as requested
		
		Value res = navigator.Eval(R"(
			return (function() {
				const items = [];
				
				function add(t, u, s) {
					if(t && u && t.length > 5) {
						items.push({ title: t.trim(), url: u, summary: s || "" });
					}
				}
				
				const selectors = [
					'h2[class*="Article_title"] > a', // ZeroHedge
					'.titleline > a', // HN
					'h3.fxs_article_title a', // FXStreet
					'a[data-test="article-title-link"]', // Investing
					'article.articleItem a.title', // Investing fallback
					'.flexposts__item-title a', // ForexFactory
					'h2.entry-title a', // GlobalResearch / NaturalNews
					'.story-link', 
					'article h2 a', 
					'div[class*="title"] a',
                    'a[class*="title"]'
				];
				
				const candidates = document.querySelectorAll(selectors.join(','));
				for(let i=0; i<candidates.length && items.length < 50; i++) {
					const el = candidates[i];
					const title = (el.innerText || el.textContent || "").trim();
					const url = el.href;
					
					if(title.length < 10) continue;
					if(!items.some(x => x.url === url))
						add(title, url, "");
				}
                
                if(items.length === 0) {
                    const msgs = document.querySelectorAll('li[class*="message"], div[class*="messageContent"]');
                    for(let i=0; i<msgs.length; i++) {
                        const txt = (msgs[i].innerText || msgs[i].textContent || "").trim();
                        if(txt.length > 20) {
                            items.push({ title: txt.substring(0, 80) + "...", url: window.location.href + "#" + i, summary: txt });
                        }
                    }
                }
				return items;
			})()
		)");
		
		if (res.Is<ValueArray>()) {
			ValueArray out;
			for(const Value& v : (ValueArray)res) {
				ValueMap m = v;
				m.Add("source", name);
				out.Add(m);
			}
			Cout() << name << ": Extracted " << out.GetCount() << " items.\n";
			return out;
		}
		
	} catch (const Exc& e) {
		Cout() << name << ": Scrape error: " << e << "\n";
	}
	return ValueArray();
}

END_UPP_NAMESPACE
