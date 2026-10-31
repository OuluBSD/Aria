#ifndef _Aria_GitHubScraper_h_
#define _Aria_GitHubScraper_h_

class AriaNavigator;
class SiteManager;

class GitHubScraper {
	AriaNavigator& navigator;
	SiteManager&   sm;
	String         site_name;
	String         owner;
	String         repo;
	String         output_dir;

	bool EnsureRepoTab(const String& path = "");

public:
	GitHubScraper(AriaNavigator& navigator, SiteManager& sm,
	              const String& owner = "SeppoPakonen",
	              const String& repo = "ai-upp-priv",
	              const String& output_dir = "");

	void SetTarget(const String& owner, const String& repo);
	String GetOutputDir() const { return output_dir; }

	ValueMap ScrapeOverview();
	ValueMap ScrapeIssues();
	ValueMap ScrapePulls();
	ValueMap ScrapeActions();
	ValueMap ScrapeProjects();
	ValueMap ScrapeSecurity();
	ValueMap ScrapeDependencies();
	ValueMap ScrapeSettings();
	ValueMap ScrapeAgents(const String& author = "SeppoPakonen");
	ValueMap ScrapeUserProfile(const String& username = "SeppoPakonen");

	ValueMap ScrapeAll();
	ValueMap SyncWithGhCli();
	bool RunSelfTests();
};

#endif
