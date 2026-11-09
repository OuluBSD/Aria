#include "ThreadsCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

ThreadsCtrl::ThreadsCtrl() {
	Load(); // Load persistent data
	
	Add(tabs.SizePos());
	tabs.LayoutId("SubTabs");
	
	// Feed View Layout
	InitFeedTree();
	feed_tab.Add(feed_tree.SizePos());
	
	// Other Lists
	public_list.AddColumn("Author", 1);
	public_list.AddColumn("Content", 4);
	
	private_list.AddColumn("Author", 1);
	private_list.AddColumn("Content", 4);
	
	// Add Tabs
	tabs.Add(feed_tab.SizePos(), "Feed");
	tabs.Add(public_list.SizePos(), "Public Messages");
	tabs.Add(private_list.SizePos(), "Private Messages");
	tabs.Add(settings_tab.SizePos(), "Config");
	
	// Settings...
	int y = 10;
	settings_tab.Add(btnRefresh.LeftPos(10, 150).TopPos(y, 30));
	btnRefresh.SetLabel("Refresh All Data");
	btnRefresh.LayoutId("BtnRefresh");
	btnRefresh << [=] { Scrape(); };
	
	LoadData(); // Populate UI
}

ThreadsCtrl::~ThreadsCtrl() {
	if (work_thread.IsOpen()) {
		is_working = false;
		work_thread.Wait();
	}
	Store();
}

void ThreadsCtrl::InitFeedTree() {
	feed_tree.LayoutId("FeedTree");
	feed_tree.AddColumn("Author");
	feed_tree.AddColumn("Content");
	feed_tree.AddColumn("Likes").SetConvert(StdConvert());
	feed_tree.AddColumn("Replies").SetConvert(StdConvert());
	feed_tree.AddColumn("Reposts").SetConvert(StdConvert());
	
	feed_tree.ColumnWidths("2 8 1 1 1");
	feed_tree.NoRoot().ShowEmptyOpen();
	feed_tree.WhenOpen = THISBACK(OnFeedExpand);
	feed_tree.WhenExpandEmpty = THISBACK(OnFeedExpand);
}

void ThreadsCtrl::Store() {
	if (!Thread::IsMain()) return;
	StoreToFile(*this, GetStorePath());
}

void ThreadsCtrl::Load() {
	if (!Thread::IsMain()) return;
	LoadFromFile(*this, GetStorePath());
}

void ThreadsCtrl::OnFeedExpand(int id) {
	Value postIdVal = feed_tree.GetValue(id);
	if (!postIdVal.Is<String>()) return;
	String postId = postIdVal.ToString();
	if (postId.IsEmpty()) return;

	if (feed_tree.GetChildCount(id) == 0 && !is_working) {
		ScrapeThread(postId);
	}
}

void ThreadsCtrl::ScrapeThread(String postId) {
	if (is_working || !navigator || !site_manager) return;
	
	is_working = true;
	WhenStatus("Fetching thread: " + postId + "...");
	
	work_thread.Run([=] {
		try {
			ThreadsScraper scraper(*navigator, *site_manager);
			ValueArray fetched = scraper.ScrapeThread(postId);
			
			PostCallback([=] {
				if (posts.Find(postId) >= 0) {
					ThreadsPost& root = posts.Get(postId);
					root.replies.Clear();
					for (const Value& v : fetched) {
						String rid = v["id"];
						if (rid == postId || rid.Find(postId) >= 0 || postId.Find(rid) >= 0) continue;
						
						ThreadsPost& r = root.replies.Add();
						r.id = rid;
						r.author = v["author"];
						r.content = v["content"];
						r.likes = v["likes"];
						r.replies_count = v["replies"];
						r.reposts = v["reposts"];
						r.timestamp = GetUtcTime();
					}
					Store();
					LoadData();
					
					// Re-open
					for(int i = 0; i < feed_tree.GetLineCount(); i++) {
						int item_id = feed_tree.GetItemAtLine(i);
						if (feed_tree.GetValue(item_id).ToString() == postId) {
							feed_tree.Open(item_id);
							break;
						}
					}
				}
				is_working = false;
				WhenStatus("Ready");
			});
		} catch (const Exc& e) {
			PostCallback([=] {
				AriaAlert("Error fetching thread: " + e);
				is_working = false;
				WhenStatus("Ready");
			});
		}
	});
}

void ThreadsCtrl::RefreshSubTab(int tab_index) {
	if (is_working || !navigator || !site_manager) return;
	
	if (tab_index == 0) { // Feed
		Scrape();
		return;
	}
	
	is_working = true;
	WhenStatus("Refreshing Threads...");
	
	work_thread.Run([=] {
		try {
			ThreadsScraper scraper(*navigator, *site_manager);
			bool success = false;
			if (tab_index == 1) success = scraper.RefreshPublic();
			else if (tab_index == 2) success = scraper.RefreshPrivate();
			else success = scraper.Refresh(false);
			
			PostCallback([=] {
				if (success) LoadData();
				is_working = false;
				WhenStatus("Ready");
			});
		} catch (const std::exception& e) {
			PostCallback([=] {
				AriaAlert("Error refreshing: " + String(e.what()));
				is_working = false;
				WhenStatus("Ready");
			});
		}
	});
}

void ThreadsCtrl::Scrape() {
	if (is_working || !navigator || !site_manager) return;
	
	is_working = true;
	WhenStatus("Deep refreshing Threads...");
	
	work_thread.Run([=] {
		try {
			ThreadsScraper scraper(*navigator, *site_manager);
			if (scraper.Refresh(true)) {
				ValueArray fresh_data = site_manager->GetSiteData("threads", "feed");
				
				PostCallback([=] {
					bool changed = false;
					for(const Value& v : fresh_data) {
						String id = v["id"];
						if(id.IsEmpty()) continue;
						
						ThreadsPost& p = posts.GetAdd(id);
						p.id = id;
						p.author = v["author"];
						p.content = v["content"];
						p.likes = v["likes"];
						p.replies_count = v["replies"];
						p.reposts = v["reposts"];
						if(IsNull(p.timestamp)) p.timestamp = GetUtcTime();
						p.last_updated = GetUtcTime();
						changed = true;
					}
					
					if(changed) {
						Store();
						LoadData();
					}
					is_working = false;
					WhenStatus("Ready");
				});
			} else {
				PostCallback([=] {
					AriaAlert("Threads refresh failed.");
					is_working = false;
					WhenStatus("Ready");
				});
			}
		} catch (const Exc& e) {
			PostCallback([=] {
				AriaAlert("Error scraping Threads: " + e);
				is_working = false;
				WhenStatus("Ready");
			});
		}
	});
}

void ThreadsCtrl::LoadData() {
	if (!Thread::IsMain()) return;
	feed_tree.Clear();
	
	Vector<String> keys;
	for(int i = 0; i < posts.GetCount(); i++)
		keys.Add(posts.GetKey(i));

	Sort(keys, [&](const String& a, const String& b) {
		return posts.Get(a).timestamp > posts.Get(b).timestamp;
	});
	
	for(const String& k : keys) {
		const ThreadsPost& p = posts.Get(k);
		
		TreeArrayCtrl::Node n;
		n.value = p.id;
		n.CanOpen(true); 
		
		int id = feed_tree.Add(0, n);
		feed_tree.SetRowValue(id, 0, p.author);
		feed_tree.SetRowValue(id, 1, p.content);
		feed_tree.SetRowValue(id, 2, p.likes);
		feed_tree.SetRowValue(id, 3, p.replies_count);
		feed_tree.SetRowValue(id, 4, p.reposts);
		
		for(const ThreadsPost& r : p.replies) {
			int rid = feed_tree.Add(id, Image(), r.id);
			feed_tree.SetRowValue(rid, 0, r.author);
			feed_tree.SetRowValue(rid, 1, r.content);
		}
	}
}

END_UPP_NAMESPACE