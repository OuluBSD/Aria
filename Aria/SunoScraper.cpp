#include "Aria.h"

NAMESPACE_UPP

SunoScraper::SunoScraper(AriaNavigator& navigator, const String& output_dir)
	: navigator(navigator)
	, output_dir(output_dir.IsEmpty() ? GetHomeDirFile(".aria/suno") : output_dir)
{}

String SunoScraper::SafeFileName(const String& name) {
	String result;
	for(int i = 0; i < name.GetCount(); i++) {
		int c = (byte)name[i];
		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		   (c >= '0' && c <= '9') || c == '-' || c == '_')
			result.Cat((char)c);
		else if(c == ' ')
			result.Cat('_');
	}
	return result.IsEmpty() ? "untitled" : result.Left(120);
}

String SunoScraper::TrackKey(const ValueMap& track) {
	String url = track["url"];
	return url.IsEmpty() ? (String)track["title"] : url;
}

bool SunoScraper::IsSunoPage() const {
	String url = ToLower(navigator.GetCurrentURL());
	return url.StartsWith("https://suno.com/") || url.StartsWith("http://suno.com/") ||
		url.StartsWith("https://www.suno.com/") || url.StartsWith("http://www.suno.com/");
}

SunoScrapeResult SunoScraper::ScrapeCurrentTab() const {
	SunoScrapeResult result;
	if(!IsSunoPage()) return result;
	String script = LoadFile(ShareDirFile("Aria/SunoScraper.js"));
	if(script.IsEmpty()) throw SessionError("Could not load share/Aria/SunoScraper.js.");
	Value value = navigator.Eval(script);
	Value parsed = value.Is<String>() ? ParseJSON((String)value) : value;
	if(!parsed.Is<ValueMap>()) return result;
	ValueMap root = parsed;
	result.valid_page = root["playlist"].Is<ValueMap>();
	result.playlist = root["playlist"];
	result.selected = root["selected"];
	return result;
}

void SunoScraper::MergeVisited(ValueMap& playlist, const ValueMap& selected) const {
	ValueArray visited = playlist["visited_tracks"];
	String key = TrackKey(selected);
	if(key.IsEmpty()) return;
	for(int i = 0; i < visited.GetCount(); i++) {
		if(TrackKey((ValueMap)visited[i]) == key) {
			visited.Set(i, selected);
			playlist.Set("visited_tracks", visited);
			playlist.Set("visited_count", visited.GetCount());
			return;
		}
	}
	visited.Add(selected);
	playlist.Set("visited_tracks", visited);
	playlist.Set("visited_count", visited.GetCount());
}

bool SunoScraper::SaveCurrentTab() {
	SunoScrapeResult result;
	try { result = ScrapeCurrentTab(); }
	catch(const SessionError&) {
		GetAriaLogger("suno").Error("No usable existing Firefox session. Start one with 'Aria open' first.");
		return false;
	}
	if(!result.valid_page) return false;
	RealizeDirectory(output_dir);
	String title = result.playlist["title"], url = result.playlist["url"];
	bool song_page = url.Find("/song/") >= 0;
	String archive = AppendFileName(output_dir, "playlist_" + SafeFileName(title) + "_" +
		AsString(url.GetHashValue()) + ".json");
	ValueMap playlist;
	if(song_page) {
		Value current = ParseJSON(LoadFile(AppendFileName(output_dir, "playlist.json")));
		if(current.Is<ValueMap>()) playlist = current;
	}
	if(FileExists(archive)) {
		Value old = ParseJSON(LoadFile(archive));
		if(old.Is<ValueMap>()) playlist = old;
	}
	if(playlist.IsEmpty()) playlist = result.playlist;
	else if(!song_page) {
		playlist.Set("title", title);
		playlist.Set("url", url);
		playlist.Set("tracks", result.playlist["tracks"]);
	}
	playlist.Set("captured_at", GetUtcTime());
	MergeVisited(playlist, result.selected);
	if(!SaveFile(archive, AsJSON(playlist, true)) ||
	   !SaveFile(AppendFileName(output_dir, "playlist.json"), AsJSON(playlist, true))) return false;
	String selected_title = result.selected["title"];
	String selected_url = result.selected["url"];
	String suffix = selected_url.IsEmpty() ? "" : "_" + AsString(selected_url.GetHashValue());
	String path = AppendFileName(output_dir, SafeFileName(selected_title) + suffix + ".txt");
	String data = "Title: " + selected_title + "\nURL: " + selected_url + "\n";
	data << "Genre: " << (String)result.selected["genre"] << "\n\nLyrics:\n";
	data << (String)result.selected["lyrics"] << "\n";
	if(!SaveFile(path, data)) return false;
	Cout() << "Saved playlist: " << archive << "\nSaved selected track: " << path << "\n";
	Cout() << "Visited tracks: " << (int)playlist["visited_count"] << "\n";
	return true;
}

bool SunoScraper::SaveAllTracks() {
	SunoScrapeResult initial;
	for(int attempt = 0; attempt < 30; attempt++) {
		initial = ScrapeCurrentTab();
		if(initial.valid_page && !((ValueArray)initial.playlist["tracks"]).IsEmpty()) break;
		Sleep(1000);
	}
	if(!initial.valid_page) return false;
	ValueArray tracks = initial.playlist["tracks"];
	if(tracks.IsEmpty()) {
		GetAriaLogger("suno").Warning("Playlist has no loaded tracks: " + (String)initial.playlist["url"]);
		return false;
	}
	if(!SaveCurrentTab()) return false;
	int saved = 0;
	for(int i = 0; i < tracks.GetCount(); i++) {
		String url = ((ValueMap)tracks[i])["url"];
		if(url.IsEmpty()) continue;
		String script = "var a=Array.from(document.querySelectorAll('a[href*=\\\"/song/\\\"]')).find(function(x){return (x.href||'').split('?')[0]=='" + url + "';}); var r=a&&a.closest('.clip-row'); if(r){r.dispatchEvent(new MouseEvent('click',{bubbles:true,cancelable:true,view:window})); return true;} return false;";
		if(!(bool)navigator.Eval(script)) continue;
		Sleep(1200);
		String show_more = LoadFile(ShareDirFile("Aria/SunoShowMore.js"));
		if(!show_more.IsEmpty()) {
			navigator.Eval(show_more);
			Sleep(800);
		}
		if(SaveCurrentTab()) saved++;
	}
	navigator.Navigate((String)initial.playlist["url"]);
	Cout() << "Saved selected tracks: " << saved << " / " << tracks.GetCount() << "\n";
	return saved > 0;
}

bool SunoScraper::SaveAllPlaylists(const String& title_prefix) {
	navigator.Navigate("https://suno.com/me/playlists");
	Sleep(5000);
	String script = LoadFile(ShareDirFile("Aria/SunoPlaylists.js"));
	Value value = navigator.Eval(script);
	Value parsed = value.Is<String>() ? ParseJSON((String)value) : value;
	if(!parsed.Is<ValueArray>()) return false;
	ValueArray playlists = parsed;
	int processed = 0;
	for(int i = 0; i < playlists.GetCount(); i++) {
		ValueMap item = playlists[i];
		String title = item["title"], url = item["url"];
		if(!title.StartsWith(title_prefix) || url.IsEmpty()) continue;
		String archive_suffix = "_" + AsString(url.GetHashValue()) + ".json";
		String done_marker = AppendFileName(output_dir, "playlist_" + AsString(url.GetHashValue()) + ".done");
		if(FileExists(done_marker)) continue;
		String archive;
		for(FindFile ff(AppendFileName(output_dir, "playlist_*.json")); ff; ff.Next()) {
			if(ff.IsFile() && ff.GetName().EndsWith(archive_suffix)) {
				archive = ff.GetPath();
				break;
			}
		}
		if(FileExists(archive)) {
			Value old = ParseJSON(LoadFile(archive));
			if(old.Is<ValueMap>()) {
				ValueMap saved = old;
				if((int)saved["visited_count"] > 0) {
					SaveFile(done_marker, "done\n");
					continue;
				}
			}
		}
		navigator.Navigate(url);
		Sleep(3500 + Random(4500));
		if(SaveAllTracks()) {
			SaveFile(done_marker, "done\n");
			processed++;
		}
	}
	navigator.Navigate("https://suno.com/me/playlists");
	Cout() << "Processed playlists: " << processed << "\n";
	return processed > 0;
}

bool SunoScraper::SaveMissingLyrics() {
	int retried = 0;
	for(FindFile ff(AppendFileName(output_dir, "playlist_*.json")); ff; ff.Next()) {
		if(!ff.IsFile()) continue;
		Value loaded = ParseJSON(LoadFile(ff.GetPath()));
		if(!loaded.Is<ValueMap>()) continue;
		ValueMap playlist = loaded;
		String playlist_url = playlist["url"];
		if(playlist_url.Find("/playlist/") < 0) continue;
		Vector<String> missing;
		ValueArray visited = playlist["visited_tracks"];
		for(int i = 0; i < visited.GetCount(); i++) {
			ValueMap track = visited[i];
			if(((String)track["lyrics"]).IsEmpty() && !((String)track["url"]).IsEmpty())
				missing.Add(track["url"]);
		}
		if(missing.IsEmpty()) continue;
		navigator.Navigate(playlist_url);
		Sleep(4000 + Random(5000));
		for(const String& url : missing) {
			String click = "var a=Array.from(document.querySelectorAll('a[href*=\\\"/song/\\\"]')).find(function(x){return (x.href||'').split('?')[0]=='" + url + "';}); var r=a&&a.closest('.clip-row'); if(r){r.dispatchEvent(new MouseEvent('click',{bubbles:true,cancelable:true,view:window})); return true;} return false;";
			if(!(bool)navigator.Eval(click)) continue;
			Sleep(1200 + Random(1000));
			String show_more = LoadFile(ShareDirFile("Aria/SunoShowMore.js"));
			if(!show_more.IsEmpty()) navigator.Eval(show_more);
			Sleep(800 + Random(800));
			if(SaveCurrentTab()) retried++;
		}
	}
	Cout() << "Retried lyrics tracks: " << retried << "\n";
	return retried > 0;
}

END_UPP_NAMESPACE
