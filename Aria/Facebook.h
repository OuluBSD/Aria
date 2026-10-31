#ifndef _Aria_Facebook_h_
#define _Aria_Facebook_h_

struct FacebookPost : Moveable<FacebookPost> {
	String id;
	String author;
	String author_id;
	String content;
	Time   time;
	int    likes = 0;
	int    comments = 0;
	
	void Serialize(Stream& s) {
		s % id % author % author_id % content % time % likes % comments;
	}
};

struct FacebookFriend : Moveable<FacebookFriend> {
	String id;
	String name;
	String profile_url;
	
	void Serialize(Stream& s) {
		s % id % name % profile_url;
	}
};

class FacebookManager {
public:
	VectorMap<String, FacebookPost>   feed;
	VectorMap<String, FacebookFriend> friends;
	VectorMap<String, FacebookPost>   own_posts;
	
	void Load() {
		LoadFromFile(*this, ConfigFile("Facebook.bin"));
	}
	
	void Store() {
		StoreToFile(*this, ConfigFile("Facebook.bin"));
	}
	
	void Serialize(Stream& s) {
		s % feed % friends % own_posts;
	}
};

#endif