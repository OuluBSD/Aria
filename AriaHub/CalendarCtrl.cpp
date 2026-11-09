#include "CalendarCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

CalendarCtrl::CalendarCtrl() {
	LayoutId("CalendarCtrl");
	Add(list.VSizePos(0, 40).HSizePos());
	Add(btnRefresh.BottomPos(5, 30).RightPos(5, 150));
	
	list.AddColumn("Date/Time", 3);
	list.AddColumn("Title", 5);
	list.AddColumn("Duration", 2);
	list.AddColumn("Location/Link", 4);
	
	btnRefresh.SetLabel("Sync Calendar");
	btnRefresh << [=] { Scrape(); };
}

void CalendarCtrl::Scrape() {
	if (!navigator || !site_manager) return;
	
	CalendarScraper scraper(*navigator, *site_manager);
	try {
		if (scraper.Refresh()) {
			LoadData();
		} else {
			AriaAlert("Calendar sync failed. Check login.");
		}
	} catch (const Exc& e) {
		AriaAlert("Error syncing Calendar: " + e);
	}
}

void CalendarCtrl::LoadData() {
	if (!site_manager) return;
	list.Clear();
	
	Value data = site_manager->GetSiteData("google_calendar", "events");
	if (data.Is<ValueArray>()) {
		for (const Value& ev : (ValueArray)data) {
			list.Add(ev["start_time"], ev["title"], ev["duration"], ev["location"]);
		}
	}
}

END_UPP_NAMESPACE
