#ifndef _Aria_Forex_h_
#define _Aria_Forex_h_

struct ForexEvent : Moveable<ForexEvent> {
	String id;
	Time   time;
	String currency;
	String impact; // Low, Medium, High
	String name;
	String forecast;
	String previous;
	String actual;
	
	// Extended details
	String description;
	String source;
	String speaker;
	String usual_effect;
	String ff_notes;
	String why_traders_care;
	
	void Serialize(Stream& s) {
		s % id % time % currency % impact % name % forecast % previous % actual
		  % description % source % speaker % usual_effect % ff_notes % why_traders_care;
	}
};

struct ForexTrade : Moveable<ForexTrade> {
	String id;
	Time   time;
	String user;
	String symbol;
	String type; // Buy/Sell
	double price;
	String lot_size;
	String stop_loss;
	String take_profit;
	
	void Serialize(Stream& s) {
		s % id % time % user % symbol % type % price % lot_size % stop_loss % take_profit;
	}
};

struct ForexRate : Moveable<ForexRate> {
	String symbol;
	double bid;
	double ask;
	double change;
	Time   updated;
	
	void Serialize(Stream& s) {
		s % symbol % bid % ask % change % updated;
	}
};

class ForexManager {
public:
	VectorMap<String, ForexEvent> events;
	VectorMap<String, ForexTrade> trades;
	VectorMap<String, ForexRate>  rates;
	
	void Load() {
		LoadFromFile(*this, ConfigFile("Forex.bin"));
	}
	
	void Store() {
		StoreToFile(*this, ConfigFile("Forex.bin"));
	}
	
	void Serialize(Stream& s) {
		s % events % trades % rates;
	}
};

#endif