#include <Core/Core.h>
#include <Aria/Aria.h>
#include <Aria/NewsScraper.h>
#include <Aria/ForexScraper.h>
#include <Aria/FacebookScraper.h>
#include <ByteVM/ByteVM.h>

using namespace Upp;

struct Command {
	virtual ~Command() {}
	virtual String GetName() const = 0;
	virtual Vector<String> GetAliases() const { return {}; }
	virtual String GetDescription() const = 0;
	virtual void ShowHelp() const = 0;
	virtual void Execute(const Vector<String>& args) = 0;
};

void EnsureSession(AriaNavigator& nav) {
	if (!nav.ConnectToSession()) {
		Cout() << "Starting new browser session...\n";
		nav.StartSession("firefox", true);
	}
}

static PyValue builtin_wait_time(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	Sleep((int)(args[0].AsDouble() * 1000));
	return PyValue::None();
}

static PyValue builtin_exit(const Vector<PyValue>& args, void*) {
	int code = args.GetCount() >= 1 ? (int)args[0].AsInt64() : 0;
	_exit(code);
	return PyValue::None();
}

static Array<Command> sCommands;

void ExecuteCommand(const String& cmdLine) {
	Vector<String> raw_args = Split(cmdLine, ' ');
	if (raw_args.GetCount() == 0) return;
	
	String cmdName = raw_args[0];
	Vector<String> sub_args;
	for(int i = 1; i < raw_args.GetCount(); i++) sub_args.Add(raw_args[i]);
	
	Command* found = nullptr;
	for(int i = 0; i < sCommands.GetCount(); i++) {
		if (sCommands[i].GetName() == cmdName) { found = &sCommands[i]; break; }
		for(const auto& a : sCommands[i].GetAliases()) if (a == cmdName) { found = &sCommands[i]; break; }
	}
	
	if (found) {
		found->Execute(sub_args);
	} else {
		Cout() << "Unknown command: " << cmdName << "\n";
	}
}

struct ScriptCommand : Command {
	String GetName() const override { return "script"; }
	Vector<String> GetAliases() const override { return {"run", "test"}; }
	String GetDescription() const override { return "Run a ByteVM (Python) script"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI script <path_to_script.py>\n";
	}
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() < 1) { ShowHelp(); return; }
		
		String path = args[0];
		String source = LoadFile(path);
		if (source.IsEmpty()) {
			Cerr() << "Error: Could not load script " << path << "\n";
			return;
		}

		try {
			PyVM vm;
			auto& globals = vm.GetGlobals();
			
			globals.GetAdd(PyValue("wait_time")) = PyValue::Function("wait_time", builtin_wait_time);
			globals.GetAdd(PyValue("_exit")) = PyValue::Function("_exit", builtin_exit);
			globals.GetAdd(PyValue("exit")) = PyValue::Function("exit", builtin_exit);
			
			globals.GetAdd(PyValue("log")) = PyValue::Function("log", [](const Vector<PyValue>& args, void*) {
				for(int i = 0; i < args.GetCount(); i++) {
					if(i) Cout() << " ";
					Cout() << args[i].ToString();
				}
				Cout() << "\n";
				Cout().Flush();
				return PyValue::None();
			});

			globals.GetAdd(PyValue("cli")) = PyValue::Function("cli", [](const Vector<PyValue>& args, void*) {
				if(args.GetCount() >= 1) ExecuteCommand(args[0].ToString());
				return PyValue::None();
			});

			static AriaNavigator sNav; 
			globals.GetAdd(PyValue("navigate")) = PyValue::Function("navigate", [](const Vector<PyValue>& args, void*) {
				if(args.GetCount() >= 1) {
					EnsureSession(sNav);
					sNav.Navigate(args[0].ToString());
				}
				return PyValue::None();
			});
			globals.GetAdd(PyValue("eval")) = PyValue::Function("eval", [](const Vector<PyValue>& args, void*) {
				if(args.GetCount() >= 1) {
					EnsureSession(sNav);
					return PyValue::FromValue(sNav.Eval(args[0].ToString()));
				}
				return PyValue::None();
			});
			
			globals.GetAdd(PyValue("find")) = PyValue::Function("find", [](const Vector<PyValue>&, void*) { return PyValue::None(); });
			globals.GetAdd(PyValue("dump_ui")) = PyValue::Function("dump_ui", [](const Vector<PyValue>&, void*) { return PyValue(""); });

			Tokenizer tokenizer;
			tokenizer.SkipPythonComments(true);
			if (!tokenizer.Process(source, path)) {
				Cerr() << "Compilation Error: Tokenization failed\n";
				return;
			}
			tokenizer.CombineTokens();
			
			PyCompiler compiler(tokenizer.GetTokens());
			Vector<PyIR> ir;
			compiler.Compile(ir);
			
			vm.SetIR(ir);
			vm.Run();
		} catch (const Exc& e) {
			if(e.Find("EXIT:0") >= 0) return;
			Cerr() << "Runtime Error: " << e << "\n";
		}
	}
};

struct NewsCommand : Command {
	String GetName() const override { return "news"; }
	Vector<String> GetAliases() const override { return {"n"}; }
	String GetDescription() const override { return "Manage news items and scrapers"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI news <subcommand>\n\n"
		       << "subcommands:\n"
		       << "  scrape        Run scrapers for all sources\n"
		       << "  list          Show persisted news items\n"
		       << "  clear         Clear news database\n";
	}
	
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() == 0) { ShowHelp(); return; }
		
		String sub = args[0];
		String path = ConfigFile("News.bin");
		
		if (sub == "scrape") {
			AriaNavigator navigator;
			EnsureSession(navigator);
			SiteManager sm;
			NewsScraper scraper(navigator, sm);
			
			VectorMap<String, NewsItem> items;
			LoadFromFile(items, path);
			
			struct Site : Moveable<Site> {
				String name;
				String url;
				Site(const String& n, const String& u) : name(n), url(u) {}
				Site() {}
			};
			Vector<Site> sites;
			sites << Site("ZeroHedge", "https://www.zerohedge.com/")
			      << Site("HackerNews", "https://news.ycombinator.com/")
			      << Site("ForexFactory", "https://www.forexfactory.com/")
			      << Site("FXStreet", "https://www.fxstreet.com")
			      << Site("Investing", "https://www.investing.com")
			      << Site("Discord", "https://discord.com/channels/283274943754665984/1363100565890007041")
			      << Site("Discord", "https://discord.com/channels/283274943754665984/675830944371965953");
			
			for(const auto& s : sites) {
				Cout() << "Scraping " << s.name << "...\n";
				ValueArray res = scraper.ScrapeSite(s.name, s.url);
				for(const Value& v : res) {
					String u = v["url"];
					if(u.IsEmpty()) continue;
					if(items.Find(u) < 0) {
						NewsItem& n = items.Add(u);
						n.url = u;
						n.title = v["title"];
						n.source = v["source"];
						n.published = GetUtcTime();
						n.id = u;
					}
				}
			}
			StoreToFile(items, path);
			Cout() << "✓ Done. Total items: " << items.GetCount() << "\n";
		}
		else if (sub == "list") {
			bool all = args.GetCount() > 1 && args[1] == "all";
			VectorMap<String, NewsItem> items;
			if (LoadFromFile(items, path)) {
				Cout() << "Persisted News Items (" << items.GetCount() << "):\n";
				for(int i = 0; i < items.GetCount(); i++) {
					const auto& n = items[i];
					Cout() << "--------------------------------------------------\n";
					Cout() << "Source:    " << n.source << "\n";
					Cout() << "Title:     " << n.title << "\n";
					Cout() << "URL:       " << n.url << "\n";
					Cout() << "Published: " << n.published << "\n";
					
					if (!all && i >= 19) {
						Cout() << "--------------------------------------------------\n";
						Cout() << "... and " << (items.GetCount() - i - 1) << " more. Use 'list all' to see everything.\n";
						break;
					}
				}
				Cout() << "--------------------------------------------------\n";
			} else {
				Cout() << "No news data found at " << path << "\n";
			}
		}
		else if (sub == "clear") {
			FileDelete(path);
			Cout() << "News database cleared.\n";
		}
		else if (sub == "path") {
			Cout() << path << "\n";
		}
		else {
			Cout() << "Unknown news subcommand: " << sub << "\n";
		}
	}
};

struct NavigatorCommand : Command {
	String GetName() const override { return "navigator"; }
	Vector<String> GetAliases() const override { return {"nav"}; }
	String GetDescription() const override { return "Direct navigator control"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI nav <url> [eval <js>]\n";
	}
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() < 1) { ShowHelp(); return; }
		
		try {
			AriaNavigator nav;
			EnsureSession(nav);
			Cout() << "Navigated to " << args[0] << ". Waiting for load...\n";
			nav.Navigate(args[0]);
			Sleep(5000);
			
			if (args.GetCount() >= 3 && args[1] == "eval") {
				Cout() << "Evaluating: " << args[2] << "\n";
				Value res = nav.Eval(args[2]);
				Cout() << "Result: " << AsString(res) << "\n";
			}
		} catch (const Exc& e) {
			Cerr() << "Navigator Error: " << e << "\n";
		}
	}
};

struct DiscordCommand : Command {
	String GetName() const override { return "discord"; }
	Vector<String> GetAliases() const override { return {"d"}; }
	String GetDescription() const override { return "Discord interactions"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI discord [scrape]\n";
	}
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() > 0 && args[0] == "scrape") {
			AriaNavigator navigator;
			EnsureSession(navigator);
			SiteManager sm;
			DiscordScraper scraper(navigator, sm);
			Cout() << "Refreshing Discord...\n";
			if (scraper.Refresh()) {
				Cout() << "✓ Discord data refreshed.\n";
			} else {
				Cerr() << "Error: Discord refresh failed.\n";
			}
		} else {
			ShowHelp();
		}
	}
};

struct GoogleMessagesCommand : Command {
	String GetName() const override { return "messages"; }
	Vector<String> GetAliases() const override { return {"msg", "sms"}; }
	String GetDescription() const override { return "Google Messages interaction"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI messages [list]\n";
	}
	void Execute(const Vector<String>& args) override {
		SiteManager sm;
		Value data = sm.GetSiteData("google_messages", "messages");
		if (data.Is<ValueArray>()) {
			ValueArray va = data;
			Cout() << "Recent Google Messages:\n";
			for(int i = 0; i < min(va.GetCount(), 10); i++) {
				ValueMap m = va[i];
				Cout() << "[" << m["sender"] << "] " << m["text"] << " (" << m["time"] << ")\n";
			}
		} else {
			Cout() << "No Google Messages data available. Run scrape in Hub first.\n";
		}
	}
};

struct ForexCommand : Command {
	String GetName() const override { return "forex"; }
	Vector<String> GetAliases() const override { return {"fx"}; }
	String GetDescription() const override { return "Forex data and interactions"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI forex <subcommand> [--force]\n\n"
		       << "subcommands:\n"
		       << "  scrape        Run all forex scrapers (FF, Investing, Oanda)\n"
		       << "  calendar      List economic calendar events\n"
		       << "  trades        List recent trades\n"
		       << "  rates         Show live exchange rates\n\n"
		       << "options:\n"
		       << "  --force, -f   Force download even if data exists\n";
	}
	
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() == 0) { ShowHelp(); return; }
		
		bool force = false;
		Vector<String> sub_args;
		for(const String& a : args) {
			if (a == "--force" || a == "-f") force = true;
			else sub_args.Add(a);
		}
		
		if (sub_args.GetCount() == 0) { ShowHelp(); return; }
		String sub = sub_args[0];
		
		AriaNavigator navigator;
		EnsureSession(navigator);
		SiteManager sm;
		ForexScraper scraper(navigator, sm);
		if (force) scraper.SetForce(true);
		
		if (sub == "scrape") {
			Cout() << "Starting Forex Scrape...\n";
			scraper.ScrapeAll();
			Cout() << "✓ Forex scrape complete.\n";
		}
		else if (sub == "calendar") {
			scraper.Load();
			const auto& events = scraper.GetManager().events;
			Cout() << "Forex Calendar Events (" << events.GetCount() << "):\n";
			for(int i = 0; i < events.GetCount(); i++) {
				const auto& e = events[i];
				Cout() << "--------------------------------------------------\n";
				Cout() << "Time:     " << e.time << " [" << e.currency << "]\n";
				Cout() << "Event:    " << e.name << " (" << e.impact << ")\n";
				Cout() << "Actual:   " << e.actual << " / Forecast: " << e.forecast << " / Prev: " << e.previous << "\n";
			}
		}
		else if (sub == "rates") {
			scraper.Load();
			const auto& rates = scraper.GetManager().rates;
			Cout() << "Live Exchange Rates (Oanda):\n";
			Cout() << "Pair         Bid        Ask        Updated\n";
			Cout() << "--------------------------------------------------\n";
			for(int i = 0; i < rates.GetCount(); i++) {
				const auto& r = rates[i];
				Cout() << Format("%-12s %-10.5f %-10.5f %s\n", r.symbol, r.bid, r.ask, r.updated);
			}
		}
		else if (sub == "trades") {
			scraper.Load();
			const auto& trades = scraper.GetManager().trades;
			Cout() << "Recent Trades (ForexFactory):\n";
			for(int i = 0; i < trades.GetCount(); i++) {
				const auto& t = trades[i];
				Cout() << "[" << t.time << "] " << t.user << ": " << t.type << " " << t.symbol << " @ " << t.price << "\n";
			}
		}
		else {
			ShowHelp();
		}
	}
};

struct FacebookCommand : Command {
	String GetName() const override { return "facebook"; }
	Vector<String> GetAliases() const override { return {"fb"}; }
	String GetDescription() const override { return "Facebook data and interactions"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI facebook <subcommand> [args]\n\n"
		       << "subcommands:\n"
		       << "  scrape             Run all FB scrapers (feed, profile, friends)\n"
		       << "  friends            List current friends from DB\n"
		       << "  friend-feed <idx>  Dump latest feed for friend at index <idx>\n";
	}
	
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() == 0) { ShowHelp(); return; }
		String sub = args[0];
		
		AriaNavigator navigator;
		SiteManager sm;
		FacebookScraper scraper(navigator, sm);
		
		if (sub == "scrape") {
			EnsureSession(navigator);
			Cout() << "Starting Facebook Scrape...\n";
			scraper.ScrapeAll();
			Cout() << "✓ Facebook scrape complete.\n";
		}
		else if (sub == "friends") {
			scraper.Load();
			const auto& friends = scraper.GetManager().friends;
			Cout() << "Facebook Friends (" << friends.GetCount() << "):\n";
			for(int i = 0; i < friends.GetCount(); i++) {
				Cout() << "[" << i << "] " << friends[i].name << "\n";
			}
		}
		else if (sub == "friend-feed") {
			if (args.GetCount() < 2) { Cout() << "Error: Friend index required.\n"; return; }
			int idx = ScanInt(args[1]);
			scraper.Load();
			const auto& friends = scraper.GetManager().friends;
			if (idx >= 0 && idx < friends.GetCount()) {
				EnsureSession(navigator);
				Cout() << "Scraping feed for " << friends[idx].name << " (" << friends[idx].profile_url << ")...\n";
				scraper.ScrapeFriendFeed(friends[idx].profile_url);
				Cout() << "✓ Feed scraped.\n";
			} else {
				Cout() << "Error: Invalid friend index.\n";
			}
		}
		else {
			ShowHelp();
		}
	}
};

struct YouTubeCommand : Command {
	String GetName() const override { return "youtube"; }
	String GetDescription() const override { return "YouTube data and interactions"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI youtube <subcommand> [args]\n\n"
		       << "subcommands:\n"
		       << "  scrape             Scrape feed and studio analytics\n"
		       << "  videos             List extracted videos\n"
		       << "  comments <url>     Scrape comments for a video URL\n";
	}
	
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() == 0) { ShowHelp(); return; }
		String sub = args[0];
		
		AriaNavigator navigator;
		SiteManager sm;
		YouTubeScraper scraper(navigator, sm);
		
		if (sub == "scrape") {
			EnsureSession(navigator);
			Cout() << "Starting YouTube Scrape...\n";
			scraper.ScrapeAll();
			Cout() << "✓ YouTube scrape complete.\n";
		}
		else if (sub == "videos") {
			scraper.Load();
			const auto& videos = scraper.GetManager().videos;
			Cout() << "YouTube Videos (" << videos.GetCount() << "):\n";
			for(int i = 0; i < videos.GetCount(); i++) {
				Cout() << "[" << i << "] " << videos[i].title << " by " << videos[i].author << " (" << videos[i].views << ")\n";
			}
		}
		else if (sub == "comments") {
			if (args.GetCount() < 2) { Cout() << "Error: Video URL required.\n"; return; }
			EnsureSession(navigator);
			scraper.ScrapeComments(args[1]);
			Cout() << "✓ Comments scraped.\n";
		}
		else {
			ShowHelp();
		}
	}
};

struct VaultCommand : Command {
	String GetName() const override { return "vault"; }
	String GetDescription() const override { return "Manage credentials in the secure vault"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI vault <subcommand> [args]\n\n"
		       << "subcommands:\n"
		       << "  set <key> <val>    Set a credential (e.g., facebook:email)\n"
		       << "  get <key>          Get a credential value\n"
		       << "  list               List all keys\n"
		       << "  remove <key>       Remove a key\n";
	}
	
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() == 0) { ShowHelp(); return; }
		String sub = args[0];
		CredentialManager cm;
		
		if (sub == "set" && args.GetCount() >= 3) {
			cm.SetCredential(args[1], args[2]);
			Cout() << "✓ Credential set.\n";
		}
		else if (sub == "get" && args.GetCount() >= 2) {
			Cout() << cm.GetCredential(args[1]) << "\n";
		}
		else if (sub == "list") {
			for (const auto& k : cm.ListKeys()) Cout() << "- " << k << "\n";
		}
		else if (sub == "remove" && args.GetCount() >= 2) {
			if (cm.RemoveCredential(args[1])) Cout() << "✓ Removed.\n";
			else Cout() << "Error: Key not found.\n";
		}
		else {
			ShowHelp();
		}
	}
};

void MainHelp(const Array<Command>& commands) {
	Cout() << "AriaCLI - Browser Automation Command Line Interface\n\n";
	Cout() << "usage: AriaCLI <command> [args]\n\n";
	Cout() << "commands:\n";
	for(int i = 0; i < commands.GetCount(); i++) {
		String n = commands[i].GetName();
		while(n.GetCount() < 12) n << " ";
		Cout() << "  " << n << " " << commands[i].GetDescription() << "\n";
	}
	Cout() << "\nUse 'AriaCLI <command>' for help on a specific command.\n";
}

CONSOLE_APP_MAIN {
	SetConfigName("AriaHub");
	sCommands.Create<NewsCommand>();
	sCommands.Create<NavigatorCommand>();
	sCommands.Create<DiscordCommand>();
	sCommands.Create<GoogleMessagesCommand>();
	sCommands.Create<ForexCommand>();
	sCommands.Create<FacebookCommand>();
	sCommands.Create<YouTubeCommand>();
	sCommands.Create<VaultCommand>();
	sCommands.Create<ScriptCommand>();
	
	const Vector<String>& raw_args = CommandLine();
	if (raw_args.GetCount() == 0) {
		MainHelp(sCommands);
		return;
	}
	
	String cmdLine;
	for(int i = 0; i < raw_args.GetCount(); i++) {
		if (i) cmdLine << " ";
		cmdLine << raw_args[i];
	}
	
	ExecuteCommand(cmdLine);
}