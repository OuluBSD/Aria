#ifndef _AriaHub_ThreadsCtrl_h_
#define _AriaHub_ThreadsCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

struct ThreadsPost : Moveable<ThreadsPost> {
	String id;
	String author;
	String content;
	int likes = 0;
	int replies_count = 0;
	int reposts = 0;
	Time timestamp;
	Time last_updated;
	
	Vector<ThreadsPost> replies; // Nested replies
	
	void Visit(Visitor& v) {
		v("id", id)("author", author)("content", content)
		 ("likes", likes)("replies_count", replies_count)("reposts", reposts)
		 ("timestamp", timestamp)("last_updated", last_updated)
		 ("replies", replies);
	}
	
	void Serialize(Stream& s) {
		s % id % author % content % likes % replies_count % reposts % timestamp % last_updated % replies;
	}
	
	void Jsonize(JsonIO& jio) {
		jio("id", id)("author", author)("content", content)
		   ("likes", likes)("replies_count", replies_count)("reposts", reposts)
		   ("timestamp", timestamp)("last_updated", last_updated)
		   ("replies", replies);
	}
};

class ThreadsCtrl : public ServiceCtrl {
public:
	typedef ThreadsCtrl CLASSNAME;
	ThreadsCtrl();
	virtual ~ThreadsCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual void   RefreshSubTab(int tab_index) override;
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "Threads"; }
	virtual Image  GetIcon() override  { return Image(); }
	virtual int    GetActiveTab() override { return tabs.Get(); }

private:
	TabCtrl   tabs;
	
	// Single Tree View for Threads
	TreeArrayCtrl feed_tree;
	
	ParentCtrl feed_tab;
	ArrayCtrl public_list;
	ArrayCtrl private_list;
	ParentCtrl settings_tab;
	
	Button btnRefresh;
	Label lblDepth;
	EditInt scrape_depth;
	Option auto_refresh;
	
	// Persistence
	VectorMap<String, ThreadsPost> posts;
	String GetStorePath() const { return ConfigFile("Threads.bin"); }
	void Store();
	void Load();
	
	void InitFeedTree();
	void OnFeedExpand(int id);
	void ScrapeThread(String postId);
	
	Thread work_thread;
	bool   is_working = false;
};

END_UPP_NAMESPACE

#endif