#include "WhatsAppCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

WhatsAppCtrl::WhatsAppCtrl() {
	LayoutId("WhatsAppCtrl");
	Add(list.VSizePos(0, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 150));
	
	list.AddColumn("Avatar").FixedWidth(40);
	list.AddColumn("Name", 3);
	list.AddColumn("Preview", 6);
	list.AddColumn("Time", 2);
	list.AddColumn("Status", 2);
	
	btnRefresh.SetLabel("Sync All Chats");
	btnRefresh << [=] { Scrape(); };
}

void WhatsAppCtrl::Scrape() {
	if (!navigator || !site_manager) return;
	
	WhatsAppScraper scraper(*navigator, *site_manager);
	try {
		if (scraper.Refresh()) {
			LoadData();
		} else {
			AriaAlert("WhatsApp sync failed. Check login status.");
		}
	} catch (const Exc& e) {
		AriaAlert("Error syncing WhatsApp: " + e);
	}
}

void WhatsAppCtrl::LoadData() {
	if (!site_manager) return;
	list.Clear();
	Value data = site_manager->GetSiteData("whatsapp", "chats");
	if (data.Is<ValueArray>()) {
		for (const Value& chat : (ValueArray)data) {
			list.Add(Image(), chat["name"], chat["last_message"], chat["timestamp"], chat["status"]);
		}
	}
}

END_UPP_NAMESPACE
