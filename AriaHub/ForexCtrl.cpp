#include "ForexCtrl.h"

NAMESPACE_UPP

ForexCtrl::ForexCtrl() {
	Add(fx_tabs.SizePos());
	
	calendar_list.AddColumn("Time");
	calendar_list.AddColumn("Cur");
	calendar_list.AddColumn("Impact");
	calendar_list.AddColumn("Event");
	calendar_list.AddColumn("Actual");
	calendar_list.AddColumn("Forecast");
	calendar_list.AddColumn("Previous");
	calendar_list.SetLineCy(Draw::GetStdFontCy() + 4);
	
	trades_list.AddColumn("Time");
	trades_list.AddColumn("User");
	trades_list.AddColumn("Symbol");
	trades_list.AddColumn("Type");
	trades_list.AddColumn("Price");
	
	rates_list.AddColumn("Symbol");
	rates_list.AddColumn("Bid");
	rates_list.AddColumn("Ask");
	rates_list.AddColumn("Updated");
	
	fx_tabs.Add(calendar_list.SizePos(), "Calendar");
	fx_tabs.Add(trades_list.SizePos(), "Trade Feed");
	fx_tabs.Add(rates_list.SizePos(), "Live Rates");
	
	calendar_list.LayoutId("CalendarList");
	trades_list.LayoutId("TradesList");
	rates_list.LayoutId("RatesList");
}

void ForexCtrl::LoadData() {
	ForexManager fm;
	fm.Load();
	
	calendar_list.Clear();
	for(int i = 0; i < fm.events.GetCount(); i++) {
		const auto& e = fm.events[i];
		calendar_list.Add(e.time, e.currency, e.impact, e.name, e.actual, e.forecast, e.previous);
	}
	
	trades_list.Clear();
	for(int i = 0; i < fm.trades.GetCount(); i++) {
		const auto& t = fm.trades[i];
		trades_list.Add(t.time, t.user, t.symbol, t.type, t.price);
	}
	
	rates_list.Clear();
	for(int i = 0; i < fm.rates.GetCount(); i++) {
		const auto& r = fm.rates[i];
		rates_list.Add(r.symbol, r.bid, r.ask, r.updated);
	}
}

void ForexCtrl::Scrape() {
	if(!navigator || !site_manager) return;
	
	AriaNavigator* nav = navigator;
	SiteManager* sm = site_manager;
	
	Thread().Run([=] {
		ForexScraper scraper(*nav, *sm);
		scraper.SetForce(true); // From GUI refresh it's always force
		scraper.ScrapeAll();
		PostCallback(THISBACK(LoadData));
	});
}

END_UPP_NAMESPACE