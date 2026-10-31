#ifndef _Aria_NewsScraper_h_
#define _Aria_NewsScraper_h_

struct NewsItem : Moveable<NewsItem> {
	String title;
	String url;
	String source;
	String summary;
	String author;
	Time   published;
	Time   scraped_time;
	
	String id; // Unique ID (usually url hash)
	
	void Visit(Visitor& v) {
		v("title", title)("url", url)("source", source)("summary", summary)
		 ("author", author)("published", published)("scraped_time", scraped_time)("id", id);
	}
	
	void Serialize(Stream& s) {
		s % title % url % source % summary % author % published % scraped_time % id;
	}
	
	void Jsonize(JsonIO& jio) {
		jio("title", title)("url", url)("source", source)("summary", summary)
		   ("author", author)("published", published)("scraped_time", scraped_time)("id", id);
	}
	
	bool operator==(const NewsItem& b) const { return id == b.id; }
};

class NewsScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	
public:
	NewsScraper(AriaNavigator& navigator, SiteManager& sm);
	
	// Returns generic news items
	ValueArray ScrapeSite(const String& name, const String& url);
	
	// Specific parsing logic helpers
	ValueArray ParseGeneric(const String& html);
	ValueArray ParseHackerNews();
	ValueArray ParseZeroHedge();
	// ... add others
};

#endif