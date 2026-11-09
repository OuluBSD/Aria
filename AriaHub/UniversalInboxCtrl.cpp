#include "UniversalInboxCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

UniversalInboxCtrl::UniversalInboxCtrl() {
	LayoutId("InboxCtrl");
	Add(list.VSizePos(0, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 150));
	
	list.AddColumn("Service").FixedWidth(80);
	list.AddColumn("Source", 3);
	list.AddColumn("Preview", 7);
	list.AddColumn("Time", 2);
	
	list.WhenLeftDouble = THISBACK(OnDouble);
	
	btnRefresh.SetLabel("Refresh All");
	btnRefresh << [=] { Scrape(); };
}

void UniversalInboxCtrl::OnDouble() {
	if (!list.IsCursor()) return;
	int s_idx = list.Get(4);
	int t_idx = list.Get(5);
	WhenJump(s_idx, t_idx);
}

void UniversalInboxCtrl::Scrape() {
	// Normally this would trigger a global refresh, but for now we just reload from cached data
	LoadData();
}

void UniversalInboxCtrl::LoadData() {
	if (!site_manager) return;
	list.Clear();
	
	// Collect from Threads
	Value threads_feed = site_manager->GetSiteData("threads", "feed");
	if (threads_feed.Is<ValueArray>()) {
		for (const Value& p : (ValueArray)threads_feed) {
			list.Add("Threads", p["author"], p["content"], "", 1, 0); // 1: Threads tab
		}
	}
	
	// Collect from WhatsApp
	Value whatsapp_chats = site_manager->GetSiteData("whatsapp", "chats");
	if (whatsapp_chats.Is<ValueArray>()) {
		for (const Value& c : (ValueArray)whatsapp_chats) {
			list.Add("WhatsApp", c["name"], c["last_message"], c["timestamp"], 2, 0); // 2: WhatsApp tab
		}
	}
	
	// Collect from Google Messages
	Value gm_convs = site_manager->GetSiteData("google_messages", "conversations");
	if (gm_convs.Is<ValueArray>()) {
		for (const Value& c : (ValueArray)gm_convs) {
			if (c["unread"]) {
				list.Add("Messages", c["sender"], c["snippet"], c["timestamp"], 3, 0); // 3: Google Messages tab
			}
		}
	}
}

END_UPP_NAMESPACE
