#ifndef _Aria_SunoScraper_h_
#define _Aria_SunoScraper_h_

struct SunoScrapeResult {
	ValueMap playlist;
	ValueMap selected;
	bool valid_page = false;
};

class SunoScraper {
	AriaNavigator& navigator;
	String output_dir;

	static String SafeFileName(const String& name);
	static String TrackKey(const ValueMap& track);
	bool IsSunoPage() const;
	void MergeVisited(ValueMap& playlist, const ValueMap& selected) const;

public:
	SunoScraper(AriaNavigator& navigator, const String& output_dir = "");

	// Scrape the currently focused browser tab without navigating it.
	SunoScrapeResult ScrapeCurrentTab() const;

	// Save the playlist snapshot and the currently selected track.
	bool SaveCurrentTab();

	// Click and save every track visible in the current playlist.
	bool SaveAllTracks();
	bool SaveAllPlaylists(const String& title_prefix);
	bool SaveMissingLyrics();
};

#endif
