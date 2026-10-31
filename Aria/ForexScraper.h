#ifndef _Aria_ForexScraper_h_
#define _Aria_ForexScraper_h_

#include "Aria.h"

class ForexScraper {
	AriaNavigator& navigator;
	SiteManager& sm;
	ForexManager fm;
	bool force = false;

public:
	ForexScraper(AriaNavigator& nav, SiteManager& sm) : navigator(nav), sm(sm) {}

	void SetForce(bool b = true) { force = b; }

	bool ScrapeAll();
	
	// ForexFactory
	bool ScrapeFFCalendar(int days_offset = 0); // 0 = today, -7 to +7
	bool ScrapeFFTrades();
        bool ScrapeFFRates();
	
	// Investing.com
	bool ScrapeInvestingCalendar();
	
	// Oanda
	bool ScrapeOandaRates();
	
	const ForexManager& GetManager() const { return fm; }
	ForexManager& GetManager() { return fm; }
	
	void Load() { fm.Load(); }
	void Store() { fm.Store(); }
};

#endif
