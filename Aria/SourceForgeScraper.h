#ifndef _Aria_SourceForgeScraper_h_
#define _Aria_SourceForgeScraper_h_

class AriaNavigator;
class SiteManager;

class SourceForgeScraper {
	AriaNavigator& navigator;
	SiteManager&   sm;
	String         site_name;
	String         project;
	String         output_dir;

	bool EnsureTab(const String& url);

public:
	SourceForgeScraper(AriaNavigator& navigator, SiteManager& sm,
	                   const String& project = "topside",
	                   const String& output_dir = "");

	void SetProject(const String& new_project);
	String GetOutputDir() const { return output_dir; }

	ValueMap ScrapeSummary();
	ValueMap ScrapeFiles();
	ValueMap ScrapeReviews();
	ValueMap ScrapeSupport();
	ValueMap ScrapeBlog();
	ValueMap ScrapeCode(const String& branch = "stable");
	ValueMap ScrapeWiki();
	ValueMap ScrapeTickets();
	ValueMap ScrapeDiscussion();
	ValueMap ScrapeMailman();
	ValueMap ScrapeAdmin();
	ValueMap ScrapeSubproject(const String& subname = "subproject");

	ValueMap ScrapeAll();
	bool RunSelfTests();
};

#endif
