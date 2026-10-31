#ifndef _Aria_YouTube_h_
#define _Aria_YouTube_h_

#include <Core/Core.h>

struct YouTubeVideo : Moveable<YouTubeVideo> {
	String id;
	String title;
	String author;
	String views;
	Time   published;
	String url;
	
	void Serialize(Stream& s) {
		s % id % title % author % views % published % url;
	}
};

struct YouTubeComment : Moveable<YouTubeComment> {
	String id;
	String video_id;
	String author;
	String content;
	Time   time;
	
	void Serialize(Stream& s) {
		s % id % video_id % author % content % time;
	}
};

class YouTubeManager {
public:
	VectorMap<String, YouTubeVideo>   videos;
	VectorMap<String, YouTubeComment> comments;
	ValueMap                          analytics;
	
	void Load() {
		LoadFromFile(*this, ConfigFile("YouTube.bin"));
	}
	
	void Store() {
		StoreToFile(*this, ConfigFile("YouTube.bin"));
	}
	
	void Serialize(Stream& s) {
		s % videos % comments % analytics;
	}
};

#endif