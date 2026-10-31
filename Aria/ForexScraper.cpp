#include "Aria.h"

NAMESPACE_UPP

bool ForexScraper::ScrapeAll() {
	Cout() << "ForexScraper: Loading manager...\n";
	fm.Load();
	
	bool ok = true;
	ok &= ScrapeOandaRates();
	ok &= ScrapeFFTrades();
	
	for(int i = -7; i <= 7; i++) {
		ok &= ScrapeFFCalendar(i);
	}
	
	ok &= ScrapeInvestingCalendar();
	
	Cout() << "ForexScraper: Storing results...\n";
	fm.Store();
	return ok;
}

static String GetFFDateString(Time t) {
	const char *months[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
	return Format("%s%d.%d", months[t.month - 1], t.day, t.year);
}

static void BypassConsent(AriaNavigator& navigator) {
	navigator.Eval(R"(
		(function() {
			const buttons = [
				'button[class*="consent"]',
				'button[id*="consent"]',
				'.fc-cta-consent',
				'.accept-all',
				'#onetrust-accept-btn-handler'
			];
			for (const s of buttons) {
				const b = document.querySelector(s);
				if (b) { try { b.click(); return true; } catch(e) {} }
			}
			return false;
		})()
	)");
}

bool ForexScraper::ScrapeFFCalendar(int days_offset) {
	Time t = GetUtcTime() + (int64)days_offset * 24 * 3600;
	String date_str = GetFFDateString(t);
	
	if (!force) {
		// Check if we already have events for this day
		Date target_d = t;
		bool found = false;
		for(int i = 0; i < fm.events.GetCount(); i++) {
			if (Date(fm.events[i].time) == target_d && fm.events[i].source != "Investing.com") {
				found = true;
				break;
			}
		}
		if (found) {
			Cout() << "ForexScraper: FF Calendar for " << date_str << " already in DB, skipping.\n";
			return true;
		}
	}

	String url = "https://www.forexfactory.com/calendar?day=" + date_str;
	Cout() << "ForexScraper: FF Calendar Navigating to " << url << "...\n";
	navigator.Navigate(url);
	Sleep(3000);
	BypassConsent(navigator);
	Sleep(2000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			if(!window.calendarComponentStates || !window.calendarComponentStates[1]) return null;
			const data = window.calendarComponentStates[1];
			const result = [];
			if(data.days) {
				for(const day of data.days) {
					if(day.events) {
						for(const e of day.events) {
							result.push({
								id: e.id.toString(),
								name: e.name,
								currency: e.currency,
								impact: e.impactName,
								time_str: e.timeLabel,
								actual: e.actual,
								forecast: e.forecast,
								previous: e.previous,
								dateline: e.dateline
							});
						}
					}
				}
			}
			return result;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "ForexScraper: FF Calendar extracted " << va.GetCount() << " events for " << date_str << "\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String id = m["id"];
			ForexEvent& e = fm.events.GetAdd(id);
			e.id = id;
			e.currency = m["currency"];
			e.name = m["name"];
			e.actual = m["actual"];
			e.forecast = m["forecast"];
			e.previous = m["previous"];
			e.impact = m["impact"];
			int64 dl = m["dateline"];
			e.time = Time(1970, 1, 1, 0, 0, 0) + dl;
		}
	}
	
	return true;
}

bool ForexScraper::ScrapeFFTrades() {
	if (!force && fm.trades.GetCount() > 0) {
		// Check freshness (last 15 mins)
		Time last_t = Null;
		for(int i = 0; i < fm.trades.GetCount(); i++)
			if (fm.trades[i].time > last_t) last_t = fm.trades[i].time;
		
		if (GetUtcTime() - last_t < 15 * 60) {
			Cout() << "ForexScraper: FF Trades recently updated, skipping.\n";
			return true;
		}
	}

	Cout() << "ForexScraper: FF Trades Navigating to https://www.forexfactory.com/trades ...\n";
	navigator.Navigate("https://www.forexfactory.com/trades");
	Sleep(3000);
	BypassConsent(navigator);
	
	navigator.Eval(R"(
		(function() {
			let count = 0;
			const itv = setInterval(() => {
				window.scrollTo(0, 1000);
				if(document.querySelectorAll('.trades_activity__row').length > 0 || count++ > 20) {
					clearInterval(itv);
				}
			}, 1000);
		})()
	)");
	Sleep(15000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const trades = [];
			const rows = document.querySelectorAll('.trades_activity__row');
			for(const row of rows) {
				const id = row.id;
				if(!id) continue;
				
				const type = (row.querySelector('.trades_activity__direction')?.innerText || "").trim();
				const info = (row.querySelector('.trades_activity__info')?.innerText || "").trim();
				const user = (row.querySelector('.usernamedisplay__username')?.innerText || "").trim();
				const overview = (row.querySelector('.trades_activity__overview')?.innerText || "").trim();
				
				trades.push({
					id: id,
					user: user,
					type: type,
					info: info,
					overview: overview
				});
			}
			return trades;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "ForexScraper: FF Trades extracted " << va.GetCount() << " trades.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String id = m["id"];
			ForexTrade& t = fm.trades.GetAdd(id);
			t.id = id;
			t.user = m["user"];
			t.type = m["type"];
			
			String overview = m["overview"];
			Vector<String> parts = Split(overview, ' ');
			if(parts.GetCount() >= 2) {
				t.symbol = parts[0];
				t.price = ScanDouble(parts[parts.GetCount()-1]);
			} else {
				t.symbol = m["info"];
				t.price = 0.0;
			}
			t.time = GetUtcTime();
		}
	}
	
	return true;
}

bool ForexScraper::ScrapeInvestingCalendar() {
	if (!force) {
		Date today = GetUtcTime();
		bool found = false;
		for(int i = 0; i < fm.events.GetCount(); i++) {
			if (Date(fm.events[i].time) == today && fm.events[i].source == "Investing.com") {
				found = true;
				break;
			}
		}
		if (found) {
			Cout() << "ForexScraper: Investing Calendar recently updated, skipping.\n";
			return true;
		}
	}

	Cout() << "ForexScraper: Investing Calendar Navigating to https://www.investing.com/economic-calendar ...\n";
	navigator.Navigate("https://www.investing.com/economic-calendar");
	Sleep(3000);
	BypassConsent(navigator);
	Sleep(5000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			if(!window.__NEXT_DATA__ || !window.__NEXT_DATA__.props || !window.__NEXT_DATA__.props.pageProps) return null;
			const state = window.__NEXT_DATA__.props.pageProps.state;
			if(!state || !state.economicCalendarStore || !state.economicCalendarStore.calendarEventsByDate) return null;
			const store = state.economicCalendarStore;
			
			const result = [];
			for(const date in store.calendarEventsByDate) {
				const events = store.calendarEventsByDate[date];
				for(const e of events) {
					result.push({
						id: "inv_" + e.occurrenceId,
						name: e.event,
						currency: e.currency,
						impact: e.importance === "3" ? "High" : (e.importance === "2" ? "Medium" : "Low"),
						time_iso: e.time,
						actual: e.actual,
						forecast: e.forecast,
						previous: e.previous,
						source: "Investing.com"
					});
				}
			}
			return result;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "ForexScraper: Investing extracted " << va.GetCount() << " events.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String id = m["id"];
			ForexEvent& e = fm.events.GetAdd(id);
			e.id = id;
			e.currency = m["currency"];
			e.name = m["name"];
			e.actual = m["actual"];
			e.forecast = m["forecast"];
			e.previous = m["previous"];
			e.impact = m["impact"];
			e.source = m["source"];
			e.time = GetUtcTime(); 
		}
	}
	
	return true;
}

bool ForexScraper::ScrapeOandaRates() {
	if (!force && fm.rates.GetCount() > 0) {
		Time last_t = Null;
		for(int i = 0; i < fm.rates.GetCount(); i++)
			if (fm.rates[i].updated > last_t) last_t = fm.rates[i].updated;
		
		if (GetUtcTime() - last_t < 5 * 60) { // 5 mins for rates
			Cout() << "ForexScraper: Oanda Rates recently updated, skipping.\n";
			return true;
		}
	}

	Cout() << "ForexScraper: Oanda Rates Navigating to https://www.oanda.com/currency-converter/live-exchange-rates/#Majors%20Pairs ...\n";
	navigator.Navigate("https://www.oanda.com/currency-converter/live-exchange-rates/#Majors%20Pairs");
	Sleep(10000);
	
	Value res = navigator.Eval(R"(
		return (function() {
			const rates = [];
			const rows = document.querySelectorAll('tr, div[class*="RateRow"]');
			
			for(const row of rows) {
				const text = row.innerText || "";
				if(text.includes('/') && (text.includes('.') || text.includes(','))) {
					const cells = row.querySelectorAll('td, div[class*="Cell"]');
					if(cells.length >= 3) {
						const symbol = (cells[0].innerText || "").trim();
						const bid = (cells[1].innerText || "").trim();
						const ask = (cells[2].innerText || "").trim();
						if(symbol.includes('/') && symbol.length < 10) {
							rates.push({ symbol: symbol, bid: bid, ask: ask });
						}
					}
				}
			}
			return rates;
		})()
	)");
	
	if(res.Is<ValueArray>()) {
		ValueArray va = res;
		Cout() << "ForexScraper: Oanda extracted " << va.GetCount() << " rates.\n";
		for(int i = 0; i < va.GetCount(); i++) {
			ValueMap m = va[i];
			String sym = m["symbol"];
			ForexRate& r = fm.rates.GetAdd(sym);
			r.symbol = sym;
			String bid_str = m["bid"].ToString();
			bid_str.Replace(",", "");
			r.bid = ScanDouble(bid_str);
			String ask_str = m["ask"].ToString();
			ask_str.Replace(",", "");
			r.ask = ScanDouble(ask_str);
			r.updated = GetUtcTime();
		}
	}
	
	return true;
}


bool ForexScraper::ScrapeFFRates() {
        String current_url;
        try {
                current_url = navigator.GetCurrentURL();
        } catch(...) {}

        if (current_url.Find("forexfactory.com") < 0) {
                Cout() << "ForexScraper: FF Rates Navigating to https://www.forexfactory.com/ ...\n";
                navigator.Navigate("https://www.forexfactory.com/");
                Sleep(3000);
                BypassConsent(navigator);
                Sleep(2000);
        } else {
                Cout() << "ForexScraper: FF Rates already on forexfactory.com, refreshing...\n";
                navigator.Eval("window.location.reload()");
                Sleep(3000);
        }

        Value res = navigator.Eval(R"(
                return (function() {
                        const result = [];
                        
                        // Try component states first (modern FF)
                        if(window.marketComponentStates && window.marketComponentStates[1]) {
                                const data = window.marketComponentStates[1];
                                if(data.instruments) {
                                        for(const inst of data.instruments) {
                                                result.push({
                                                        symbol: inst.symbol,
                                                        bid: inst.bid,
                                                        ask: inst.ask,
                                                        change: inst.change
                                                });
                                        }
                                        if(result.length > 0) return result;
                                }
                        }

                        // Fallback to DOM scraping
                        const rows = document.querySelectorAll('.market__row, .market-table tr');
                        for(const row of rows) {
                                const symbolEl = row.querySelector('.market__symbol, .symbol');
                                const bidEl = row.querySelector('.market__bid, .bid');
                                const askEl = row.querySelector('.market__ask, .ask');
                                
                                if(symbolEl && bidEl && askEl) {
                                        result.push({
                                                symbol: symbolEl.innerText.trim(),
                                                bid: bidEl.innerText.trim(),
                                                ask: askEl.innerText.trim()
                                        });
                                }
                        }
                        return result;
                })()
        )");

        if(res.Is<ValueArray>()) {
                ValueArray va = res;
                Cout() << "ForexScraper: FF extracted " << va.GetCount() << " rates.\n";
                for(int i = 0; i < va.GetCount(); i++) {
                        ValueMap m = va[i];
                        String sym = m["symbol"];
                        if(sym.IsEmpty()) continue;
                        
                        ForexRate& r = fm.rates.GetAdd(sym);
                        r.symbol = sym;
                        r.bid = ScanDouble(m["bid"].ToString());
                        r.ask = ScanDouble(m["ask"].ToString());
                        r.change = ScanDouble(m["change"].ToString());
                        r.updated = GetUtcTime();
                }
                return va.GetCount() > 0;
        }

        return false;
}

END_UPP_NAMESPACE
