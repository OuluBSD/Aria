#include "Aria.h"
#include <plugin/pcre/Pcre.h>

NAMESPACE_UPP

String GeminiProvider::Generate(const String& prompt, const String& context, const String& output_format) {
	static Mutex gemini_mutex;
	Mutex::Lock __(gemini_mutex);
	
	String full_prompt = prompt;
	if (!context.IsEmpty()) {
		full_prompt = "Context:\n" + context + "\n\nUser Request: " + prompt;
	}
	
	if (output_format == "json") {
		full_prompt << "\n\nIMPORTANT: Return ONLY valid JSON. Do not include markdown code blocks or any other text.";
	} else if (output_format == "markdown") {
		full_prompt << "\n\nIMPORTANT: Use Markdown formatting.";
	}
	
	String gemini_cli = GetHomeDirFile("node_modules/.bin/gemini");
	if (!FileExists(gemini_cli)) {
		return "Error: Gemini CLI not found at " + gemini_cli;
	}
	
	String cmd = gemini_cli + " -y --model gemini-3-flash-preview -o stream-json -";
	
	LocalProcess p;
	if (!p.Start2(cmd)) return "Error starting Gemini CLI";
	
	p.Write(full_prompt);
	p.CloseWrite();
	
	String stdout_str, stderr_str;
	while (p.IsRunning()) {
		String o, e;
		if (p.Read2(o, e)) {
			stdout_str << o;
			stderr_str << e;
		}
		Sleep(1);
	}
	
	if (p.GetExitCode() != 0) {
		return "Gemini CLI error: " + stderr_str;
	}
	
	String response_text;
	Vector<String> lines = Split(stdout_str, '\n');
	for (const String& line : lines) {
		if (TrimBoth(line).IsEmpty()) continue;
		Value v = ParseJSON(line);
		if (v.Is<ValueMap>()) {
			ValueMap vm = v;
			if (vm["type"] == "message" && vm["role"] == "assistant") {
				response_text << (String)vm["content"];
			}
		}
	}
	
	String text = TrimBoth(response_text);
	if (output_format == "json") {
		if (text.StartsWith("```json")) {
			text = text.Mid(7);
			int q = text.ReverseFind("```");
			if (q >= 0) text = text.Left(q);
		} else if (text.StartsWith("```")) {
			text = text.Mid(3);
			int q = text.ReverseFind("```");
			if (q >= 0) text = text.Left(q);
		}
	}
	
	return TrimBoth(text);
}

String OtpManager::ExtractOtp(const String& text) {
	// Look for 4-8 digit codes, often surrounded by boundaries
	RegExp re("(\\b\\d{4,8}\\b)");
	if (re.GlobalMatch(text)) {
		return re[0];
	}
	return "";
}

Aria::Aria() {
	plugin_manager.LoadPlugins();
	navigator.Create();
	// Use default gemini provider
	static GeminiProvider default_gemini;
	navigator->SetAIProvider(&default_gemini);
}

void Aria::Run(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.SetDescription("Aria CLI - Your web automation assistant.");
	cla.AddArg("log-level", 0, "Set the logging level.", true, "{DEBUG,INFO,WARNING,ERROR,CRITICAL}");
	cla.AddArg("json-logs", 0, "Use JSON format for file logging.", false);
	cla.AddArg("trace-id", 0, "Optional trace ID for this execution.", true, "TRACE_ID");
	cla.AddArg("force", 0, "Force actions and bypass safety warnings.", false);
	cla.AddArg("slow-mo", 0, "Add a delay in seconds between browser actions.", true, "SLOW_MO");
	cla.AddArg("provider", 0, "The AI provider to use for generation.", true, "PROVIDER");
	cla.AddArg("navigator", 0, "The navigator engine to use.", true, "NAVIGATOR");
	cla.AddArg("json", 0, "Emit scraper output as JSON.", false);
	cla.AddArg("version", 'v', "Show version information.", false);
	cla.AddArg("help", 'h', "show this help message and exit", false);
	
	cla.SetStopOnPositional();
	cla.AddPositional("{help,open,close,page,script,report,site,diag,settings,man,tutorial,version,security,vault,test,plugin}");
	cla.AddHelpText("    help                Show this help message and exit.");
	cla.AddHelpText("    open                Open a browser instance.");
	cla.AddHelpText("    close               Close browser instances.");
	cla.AddHelpText("    page                Manage browser pages.");
	cla.AddHelpText("    script              Manage automation scripts.");
	cla.AddHelpText("    report              Manage local reports.");
	cla.AddHelpText("    site                Manage site-specific data and scraping.");
	cla.AddHelpText("    diag                Show diagnostic information.");
	cla.AddHelpText("    settings            Show current configuration.");
	cla.AddHelpText("    man                 Show manual pages.");
	cla.AddHelpText("    tutorial            Start the interactive tutorial.");
	cla.AddHelpText("    version             Show version information.");
	cla.AddHelpText("    security            Show security best practices.");
	cla.AddHelpText("    vault               Manage credentials.");
	cla.AddHelpText("    test                Run diagnostic tests.");
	cla.AddHelpText("    plugin              Manage plugins.");
	
	bool parsed = cla.Parse(args);
	if (!parsed || cla.IsArg("help") || (cla.GetPositionalCount() == 0 && !cla.IsArg("version"))) {
		cla.PrintHelp();
		return;
	}
	
	if (cla.IsArg("version")) {
		Cout() << "Aria CLI version 0.1.0\n";
		return;
	}

	String cmd = cla.GetPositional(0);
	Vector<String> rest = cla.GetRest();
	
	if (cmd == "version") {
		Cout() << "Aria CLI version 0.1.0\n";
	} else if (cmd == "open") {
		CommandLineArguments sub;
		sub.SetDescription("Open a browser instance.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddArg("headless", 0, "Run in headless mode", false);
		sub.AddArg("profile", 0, "Use default browser profile (default)", false);
		sub.AddArg("profile-path", 0, "Path to custom browser profile", true, "PATH");
		sub.AddArg("no-profile", 0, "Do not use browser profile", false);
		sub.AddArg("browser", 0, "Browser to use (default: firefox)", true, "BROWSER");
		sub.AddPositional("url", "URL to open", STRING_V, "");

		if (!sub.Parse(rest) || sub.IsArg("help")) { sub.PrintHelp(); return; }

		safety_manager.EnsureDisclaimerAccepted();

		String browser = sub.GetArg("browser");
		if (browser.IsEmpty()) browser = "firefox";
		String url = sub.GetPositional(0);
		if (!url.IsEmpty() && !url.StartsWith("http://") && !url.StartsWith("https://") && !url.StartsWith("about:"))
			url = "https://" + url;
		bool headless = sub.IsArg("headless");
		bool use_profile = !sub.IsArg("no-profile");
		String custom_profile = sub.GetArg("profile-path");
		
		navigator->SetAutoCloseSession(false);
		
		bool reconnected = false;
		if (use_profile && custom_profile.IsEmpty() && navigator->ConnectToSession(browser)) {
			if (!headless && navigator->SessionIsHeadless(browser)) {
				//A visible window was requested but the live session has none: drop it and
				//start a fresh, visible browser instead of silently attaching to a headless one.
				GetAriaLogger("navigator").Warning("Active " + browser
					+ " session has no visible window; restarting to open one.");
				navigator->CloseSession(browser);
			} else {
				GetAriaLogger("navigator").Info("Connected to active " + browser + " session.");
				reconnected = true;
			}
		}
		
		if (!reconnected) {
			navigator->StartSession(browser, headless, use_profile, custom_profile);
		}
		
		if (!url.IsEmpty()) {
			if (safety_manager.CheckUrlSafety(url))
				navigator->Navigate(url);
		}
	} else if (cmd == "close") {
		navigator->CloseSession();
	} else if (cmd == "page") {
		CommandLineArguments sub;
		sub.SetDescription("Manage browser pages.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list,new,source,eval,network,info,errors}", "Command to perform");
		sub.AddPositional("arg", "Command argument", STRING_V, "");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String p_cmd = sub.GetPositional(0);
		if (p_cmd == "list") {
			ValueArray tabs = navigator->ListTabs();
			if (tabs.IsEmpty()) Cout() << "No active tabs.\n";
			else {
				for (int i = 0; i < tabs.GetCount(); i++)
					Cout() << i << ": " << AsJSON(tabs[i]) << "\n";
			}
		} else if (p_cmd == "new") {
			String url = sub.GetPositionalCount() > 1 ? (String)sub.GetPositional(1) : "about:blank";
			navigator->NewTab(url);
		} else if (p_cmd == "source") {
			Cout() << navigator->GetPageContent() << "\n";
		} else if (p_cmd == "eval") {
			if (sub.GetPositionalCount() < 2) { Cout() << "Error: eval requires JS code.\n"; return; }
			Cout() << AsJSON(navigator->Eval(sub.GetPositional(1))) << "\n";
		} else if (p_cmd == "network") {
			String filter = sub.GetPositionalCount() > 1 ? (String)sub.GetPositional(1) : "";
			String js = Format(
				"let f = '%s';"
				"let entries = window.performance.getEntriesByType('resource');"
				"if(f) entries = entries.filter(e => e.name.includes(f));"
				"return JSON.stringify(entries.map(e => ({"
				"  name: e.name,"
				"  type: e.initiatorType,"
				"  duration: Math.round(e.duration),"
				"  transferSize: e.transferSize || 0"
				"})));",
				filter
			);
			Value v = navigator->Eval(js);
			Cout() << AsJSON(v.Is<String>() ? ParseJSON((String)v) : v, true) << "\n";
		} else if (p_cmd == "info") {
			String js =
				"return JSON.stringify({"
				"  url: window.location.href,"
				"  title: document.title,"
				"  readyState: document.readyState,"
				"  forms: document.forms.length,"
				"  buttons: document.querySelectorAll('button').length,"
				"  inputs: document.querySelectorAll('input,textarea').length,"
				"  links: document.querySelectorAll('a').length"
				"});";
			Value v = navigator->Eval(js);
			Cout() << AsJSON(v.Is<String>() ? ParseJSON((String)v) : v, true) << "\n";
		} else if (p_cmd == "errors") {
			String js =
				"return JSON.stringify({"
				"  ariaErrors: (window.__aria_errors || []),"
				"  lastError: (window.__last_error || null)"
				"});";
			Value v = navigator->Eval(js);
			Cout() << AsJSON(v.Is<String>() ? ParseJSON((String)v) : v, true) << "\n";
		}
	} else if (cmd == "script") {
		CommandLineArguments sub;
		sub.SetDescription("Manage automation scripts.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list,create,run}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String s_cmd = sub.GetPositional(0);
		Vector<String> s_rest = sub.GetRest();
		
		if (s_cmd == "list") {
			for (const auto& s : script_manager.ListScripts())
				Cout() << s.id << ": " << s.name << " - " << s.prompt.Left(50) << "...\n";
		} else if (s_cmd == "create") {
			if (s_rest.GetCount() < 1) { Cout() << "Usage: script create <prompt> [name]\n"; return; }
			String prompt = s_rest[0];
			String name = s_rest.GetCount() > 1 ? s_rest[1] : "";
			int id = script_manager.CreateScript(prompt, name);
			Cout() << "Created script " << id << "\n";
		} else if (s_cmd == "run") {
			if (s_rest.GetCount() < 1) { Cout() << "Usage: script run <id_or_name> [k=v...]\n"; return; }
			String identifier = s_rest[0];
			ValueMap params;
			for (int i = 1; i < s_rest.GetCount(); i++) {
				Vector<String> kv = Split(s_rest[i], '=');
				if (kv.GetCount() == 2) params.Set(kv[0], kv[1]);
			}
			Cout() << "Running script: " << identifier << "\n";
			try {
				script_manager.RunScript(identifier, ~navigator, params);
			} catch (const Exc& e) {
				Cout() << "Script Error: " << e << "\n";
			}
		}
	} else if (cmd == "site") {
		CommandLineArguments sub;
		sub.SetDescription("Manage site-specific data and scraping.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list,discord,slack,whatsapp,messages,calendar,youtube,threads,suno,tyomarkkinatori,duunitori,managerskylark,manager,github,sourceforge,sf}", "Command to perform");
		sub.AddHelpText("    suno                Save the active Suno playlist and selected track.");
		sub.AddHelpText("    tyomarkkinatori     Scrape Työmarkkinatori job vacancies.");
		sub.AddHelpText("    duunitori           Scrape Duunitori job vacancies.");
		sub.AddHelpText("    managerskylark      Inspect, scrape, and test ManagerSkylark and WebChat.");
		sub.AddHelpText("    github              Inspect, scrape, and test GitHub repository & platform parity.");
		sub.AddHelpText("    sourceforge         Inspect, scrape, and test SourceForge project & platform parity.");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String site_cmd = sub.GetPositional(0);
		Vector<String> s_rest = sub.GetRest();
		
		if (site_cmd == "list") {
			for (const String& s : site_manager.ListSites()) Cout() << "- " << s << "\n";
		} else if (site_cmd == "discord") {
			CommandLineArguments discord;
				discord.AddArg("json", 0, "Emit a versioned JSON envelope.", false);
				discord.AddArg("no-theme", 0, "Do not include sanitized computed theme metadata.", false);
				discord.AddArg("incremental", 0, "Read currently rendered/new messages without historical scrolling.", false);
				discord.AddArg("refresh", 0, "Perform a full guild/channel history refresh.", false);
			discord.AddArg("help", 'h', "show this help message and exit", false);
			discord.AddPositional("{channel,guild}", "Discord resource to scrape");
			discord.AddPositional("url", "Discord channel or guild channel URL", STRING_V, "");
			if (!discord.Parse(s_rest) || discord.IsArg("help") || discord.GetPositionalCount() < 2 ||
				(discord.GetPositional(0) != "channel" && discord.GetPositional(0) != "guild")) {
				discord.PrintHelp();
				return;
			}
			String channel_url = discord.GetPositional(1);
			DiscordScraper scraper(*navigator, site_manager);
			if (discord.IsArg("json")) {
				if (discord.IsArg("incremental") && discord.IsArg("refresh")) throw Exc("--incremental and --refresh are mutually exclusive");
				ValueMap output = discord.GetPositional(0) == "guild" ? scraper.ScrapeGuild(channel_url, discord.IsArg("no-theme"), discord.IsArg("incremental")) : scraper.ScrapeChannel(channel_url, discord.IsArg("no-theme"));
				Cout() << AsJSON(output) << "\n";
			} else {
				Cout() << "Scraping Discord " << discord.GetPositional(0) << ": " << channel_url << "\n";
				if (discord.IsArg("incremental") && discord.IsArg("refresh")) throw Exc("--incremental and --refresh are mutually exclusive");
				ValueMap output = discord.GetPositional(0) == "guild" ? scraper.ScrapeGuild(channel_url, discord.IsArg("no-theme"), discord.IsArg("incremental")) : scraper.ScrapeChannel(channel_url, discord.IsArg("no-theme"));
				if (!IsNull(output["error"]))
					Cout() << "Discord scrape failed.\n";
			}
		} else if (site_cmd == "slack") {
			CommandLineArguments slack;
			slack.AddArg("json", 0, "Emit a versioned JSON envelope.", false);
			slack.AddArg("help", 'h', "show this help message and exit", false);
			slack.AddPositional("channel", "Channel name; defaults to ARIA_SLACK_CHANNELS or the standard channels", STRING_V, "");
			if (!slack.Parse(s_rest) || slack.IsArg("help")) { slack.PrintHelp(); return; }
			Vector<String> channels;
			if (slack.GetPositionalCount() > 0 && !((String)slack.GetPositional(0)).IsEmpty()) channels.Add(slack.GetPositional(0));
			SlackScraper scraper(*navigator, site_manager);
			ValueMap output = scraper.Scrape(channels);
			if (slack.IsArg("json") || cla.IsArg("json")) Cout() << AsJSON(output) << "\n";
			else Cout() << "Slack scrape: " << AsJSON(output["diagnostics"]) << "\n";
		} else if (site_cmd == "whatsapp") {
			WhatsAppScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing WhatsApp data...\n";
			scraper.Refresh();
		} else if (site_cmd == "messages") {
			GoogleMessagesScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing Google Messages data...\n";
			scraper.Refresh();
		} else if (site_cmd == "calendar") {
			CalendarScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing Google Calendar data...\n";
			scraper.Refresh();
		} else if (site_cmd == "youtube") {
			YouTubeStudioScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing YouTube Studio data...\n";
			scraper.Refresh();
		} else if (site_cmd == "threads") {
			ThreadsScraper scraper(*navigator, site_manager);
			Cout() << "Refreshing Threads data...\n";
			scraper.Refresh();
		} else if (site_cmd == "suno") {
			bool retry_lyrics = s_rest.GetCount() > 0 && s_rest[0] == "retry-lyrics";
			bool all_playlists = !retry_lyrics && s_rest.GetCount() > 0 && s_rest[0] == "playlists";
			bool all_tracks = !retry_lyrics && !all_playlists && s_rest.GetCount() > 0 && s_rest[0] == "all";
			int output_index = retry_lyrics ? 1 : (all_playlists ? 2 : (all_tracks ? 1 : 0));
			String output_dir = s_rest.GetCount() > output_index ? s_rest[output_index] : "";
			navigator->SetAutoCloseSession(false);
			SunoScraper scraper(*navigator, output_dir);
			bool ok = retry_lyrics ? scraper.SaveMissingLyrics() :
				(all_playlists ? scraper.SaveAllPlaylists(s_rest.GetCount() > 1 ? s_rest[1] : "") :
				(all_tracks ? scraper.SaveAllTracks() : scraper.SaveCurrentTab()));
			if (!ok)
				Cout() << "Suno scrape failed. Keep the desired Suno playlist tab active.\n";
		} else if (site_cmd == "tyomarkkinatori") {
			String out_dir = s_rest.GetCount() > 0 ? s_rest[0] : "";
			String munic = s_rest.GetCount() > 1 ? s_rest[1] : "564";
			String query = s_rest.GetCount() > 2 ? s_rest[2] : "";
			TyomarkkinatoriScraper scraper(*navigator, site_manager, out_dir);
			Cout() << "Scraping Työmarkkinatori listings...\n";
			scraper.Scrape(munic, query);
		} else if (site_cmd == "duunitori") {
			String out_dir = s_rest.GetCount() > 0 ? s_rest[0] : "";
			String area = s_rest.GetCount() > 1 ? s_rest[1] : "oulu";
			String query = s_rest.GetCount() > 2 ? s_rest[2] : "";
			DuunitoriScraper scraper(*navigator, site_manager, out_dir);
			Cout() << "Scraping Duunitori listings...\n";
			scraper.Scrape(area, query);
		} else if (site_cmd == "managerskylark" || site_cmd == "manager") {
			String action = s_rest.GetCount() > 0 ? s_rest[0] : "status";
			String base_url = "http://127.0.0.1:8080";
			for(int i = 1; i < s_rest.GetCount(); i++) {
				if(s_rest[i].StartsWith("http://") || s_rest[i].StartsWith("https://")) {
					base_url = s_rest[i];
					break;
				}
			}
			navigator->SetAutoCloseSession(false);
			ManagerSkylarkScraper scraper(*navigator, site_manager, base_url);

			if (action == "status") {
				ValueMap st = scraper.CheckStatus();
				Cout() << AsJSON(st, true) << "\n";
			} else if (action == "overview") {
				ValueMap ov = scraper.ScrapeOverview();
				Cout() << AsJSON(ov, true) << "\n";
			} else if (action == "chat") {
				ValueMap ci = scraper.InspectChat();
				Cout() << AsJSON(ci, true) << "\n";
			} else if (action == "send" || action == "chat-send") {
				String prompt = s_rest.GetCount() > 1 ? s_rest[1] : "Explain how the WebChat streaming works";
				String role = s_rest.GetCount() > 2 ? s_rest[2] : "pair-programmer";
				Cout() << "Sending prompt to ManagerSkylark WebChat: \"" << prompt << "\" (role: " << role << ")...\n";
				ValueMap res = scraper.SendChatMessage(prompt, role, 10);
				Cout() << AsJSON(res, true) << "\n";
			} else if (action == "test") {
				if (!scraper.RunSelfTests()) {
					SetExitCode(1);
					return;
				}
			} else {
				Cout() << "Usage: Aria site managerskylark {status,overview,chat,send [prompt] [role],test} [base_url]\n";
			}
		} else if (site_cmd == "github") {
			String action = s_rest.GetCount() > 0 ? s_rest[0] : "overview";
			String owner = s_rest.GetCount() > 1 ? s_rest[1] : "SeppoPakonen";
			String repo = s_rest.GetCount() > 2 ? s_rest[2] : "ai-upp-priv";

			navigator->SetAutoCloseSession(false);
			GitHubScraper scraper(*navigator, site_manager, owner, repo);

			if (action == "overview") {
				ValueMap ov = scraper.ScrapeOverview();
				Cout() << AsJSON(ov, true) << "\n";
			} else if (action == "issues") {
				ValueMap is = scraper.ScrapeIssues();
				Cout() << AsJSON(is, true) << "\n";
			} else if (action == "pulls") {
				ValueMap pl = scraper.ScrapePulls();
				Cout() << AsJSON(pl, true) << "\n";
			} else if (action == "actions") {
				ValueMap ac = scraper.ScrapeActions();
				Cout() << AsJSON(ac, true) << "\n";
			} else if (action == "projects") {
				ValueMap pr = scraper.ScrapeProjects();
				Cout() << AsJSON(pr, true) << "\n";
			} else if (action == "security") {
				ValueMap sc = scraper.ScrapeSecurity();
				Cout() << AsJSON(sc, true) << "\n";
			} else if (action == "dependencies") {
				ValueMap dp = scraper.ScrapeDependencies();
				Cout() << AsJSON(dp, true) << "\n";
			} else if (action == "settings") {
				ValueMap st = scraper.ScrapeSettings();
				Cout() << AsJSON(st, true) << "\n";
			} else if (action == "agents") {
				ValueMap ag = scraper.ScrapeAgents(owner);
				Cout() << AsJSON(ag, true) << "\n";
			} else if (action == "user" || action == "profile") {
				ValueMap pf = scraper.ScrapeUserProfile(owner);
				Cout() << AsJSON(pf, true) << "\n";
			} else if (action == "sync" || action == "cli") {
				ValueMap cl = scraper.SyncWithGhCli();
				Cout() << AsJSON(cl, true) << "\n";
			} else if (action == "scrape-all" || action == "all") {
				ValueMap al = scraper.ScrapeAll();
				Cout() << AsJSON(al, true) << "\n";
			} else if (action == "test") {
				if (!scraper.RunSelfTests()) {
					SetExitCode(1);
					return;
				}
			} else {
				Cout() << "Usage: Aria site github {overview,issues,pulls,actions,projects,security,dependencies,settings,agents,user,scrape-all,sync,test} [owner] [repo]\n";
			}
		} else if (site_cmd == "sourceforge" || site_cmd == "sf") {
			String action = s_rest.GetCount() > 0 ? s_rest[0] : "summary";
			String project = s_rest.GetCount() > 1 ? s_rest[1] : "topside";

			navigator->SetAutoCloseSession(false);
			SourceForgeScraper scraper(*navigator, site_manager, project);

			if (action == "summary") {
				ValueMap sm = scraper.ScrapeSummary();
				Cout() << AsJSON(sm, true) << "\n";
			} else if (action == "files") {
				ValueMap fl = scraper.ScrapeFiles();
				Cout() << AsJSON(fl, true) << "\n";
			} else if (action == "reviews") {
				ValueMap rv = scraper.ScrapeReviews();
				Cout() << AsJSON(rv, true) << "\n";
			} else if (action == "support") {
				ValueMap sp = scraper.ScrapeSupport();
				Cout() << AsJSON(sp, true) << "\n";
			} else if (action == "blog") {
				ValueMap bl = scraper.ScrapeBlog();
				Cout() << AsJSON(bl, true) << "\n";
			} else if (action == "code") {
				String branch = s_rest.GetCount() > 2 ? s_rest[2] : "stable";
				ValueMap cd = scraper.ScrapeCode(branch);
				Cout() << AsJSON(cd, true) << "\n";
			} else if (action == "wiki") {
				ValueMap wk = scraper.ScrapeWiki();
				Cout() << AsJSON(wk, true) << "\n";
			} else if (action == "tickets") {
				ValueMap tk = scraper.ScrapeTickets();
				Cout() << AsJSON(tk, true) << "\n";
			} else if (action == "discussion") {
				ValueMap ds = scraper.ScrapeDiscussion();
				Cout() << AsJSON(ds, true) << "\n";
			} else if (action == "mailman") {
				ValueMap mm = scraper.ScrapeMailman();
				Cout() << AsJSON(mm, true) << "\n";
			} else if (action == "subproject") {
				String subname = s_rest.GetCount() > 2 ? s_rest[2] : "subproject";
				ValueMap sb = scraper.ScrapeSubproject(subname);
				Cout() << AsJSON(sb, true) << "\n";
			} else if (action == "admin") {
				ValueMap ad = scraper.ScrapeAdmin();
				Cout() << AsJSON(ad, true) << "\n";
			} else if (action == "scrape-all" || action == "all") {
				ValueMap al = scraper.ScrapeAll();
				Cout() << AsJSON(al, true) << "\n";
			} else if (action == "test") {
				if (!scraper.RunSelfTests()) {
					SetExitCode(1);
					return;
				}
			} else {
				Cout() << "Usage: Aria site sourceforge {summary,files,reviews,support,blog,code,wiki,tickets,discussion,mailman,subproject,admin,scrape-all,test} [project]\n";
			}
		}
	} else if (cmd == "plugin") {
		CommandLineArguments sub;
		sub.SetDescription("Manage plugins.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{list}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String p_cmd = sub.GetPositional(0);
		if (p_cmd == "list") {
			Cout() << "AI Providers:\n";
			for (const String& p : plugin_manager.ListAIProviders())
				Cout() << "- " << p << "\n";
			Cout() << "- gemini (built-in)\n";
		}
	} else if (cmd == "vault") {
		CommandLineArguments sub;
		sub.SetDescription("Manage credentials.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{set,get,list,remove}", "Command to perform");
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		CredentialManager cm;
		String v_cmd = sub.GetPositional(0);
		Vector<String> v_rest = sub.GetRest();
		
		if (v_cmd == "set") {
			if (v_rest.GetCount() < 2) { Cout() << "Usage: vault set <key> <value>\n"; return; }
			cm.SetCredential(v_rest[0], v_rest[1]);
			Cout() << "Credential set: " << v_rest[0] << "\n";
		} else if (v_cmd == "get") {
			if (v_rest.GetCount() < 1) { Cout() << "Usage: vault get <key>\n"; return; }
			Cout() << cm.GetCredential(v_rest[0]) << "\n";
		} else if (v_cmd == "list") {
			for (const auto& k : cm.ListKeys()) Cout() << "- " << k << "\n";
		} else if (v_cmd == "remove") {
			if (v_rest.GetCount() < 1) { Cout() << "Usage: vault remove <key>\n"; return; }
			if (cm.RemoveCredential(v_rest[0])) Cout() << "Credential removed.\n";
			else Cout() << "Credential not found.\n";
		}
	} else if (cmd == "test") {
		CommandLineArguments sub;
		sub.SetDescription("Run diagnostic tests.");
		sub.AddArg("help", 'h', "show this help message and exit", false);
		sub.AddPositional("{bot}", "Command to perform");
		sub.AddArg("no-headless", 0, "Disable headless mode for test", false);
		
		if (!sub.Parse(rest) || sub.IsArg("help") || sub.GetPositionalCount() == 0) { sub.PrintHelp(); return; }
		
		String test_cmd = sub.GetPositional(0);
		if (test_cmd == "bot") {
			Cout() << "Running Bot Detection Test (browserscan.net)...\n";
			
			bool headless = !sub.IsArg("no-headless");
				
			navigator->StartSession("firefox", headless);
			navigator->Navigate("https://www.browserscan.net/bot-detection");
			Cout() << "Waiting for scan to complete (15s)...\n";
			Sleep(15000);
			
			String body_text;
			try {
				body_text = navigator->GetPageContent();
				Cout() << "Page Title: " << (String)navigator->Eval("return document.title") << "\n";
			} catch (const Exc& e) {
				Cout() << "Warning: Failed to get page content: " << e << "\n";
			}
			
			// More specific detection check: Look for "Your browser is a BOT" or similar negative results
			// BrowserScan typically shows a badge or specific text if it detects automation.
			bool detected_as_bot = body_text.Find("Your browser is a BOT") >= 0 || 
			                       body_text.Find("Bot detected") >= 0;
			
			if (detected_as_bot) {
				Cout() << "❌ FAILED: Browser detected as BOT by BrowserScan\n";
			} else {
				Cout() << "✅ PASSED: No bot detection message found\n";
			}
			
			Value webdriver = navigator->Eval("return navigator.webdriver");
			Cout() << "navigator.webdriver: " << AsJSON(webdriver) << "\n";
			
			Value comp = navigator->Eval(R"(
				return {
					webdriver: navigator.webdriver,
					pluginsLength: navigator.plugins.length,
					languages: navigator.languages,
					chrome: !!window.chrome,
					permissions: !!navigator.permissions,
					hardwareConcurrency: navigator.hardwareConcurrency
				};
			)");
			Cout() << "Comprehensive Result: " << AsJSON(comp, true) << "\n";
		}
	} else if (cmd == "report") {
		Cout() << "Manage local reports (not yet implemented).\n";
	} else if (cmd == "diag") {
		Cout() << "=== Aria Diagnostic Information ===\n";
		// 1. Check WebDriver daemon (geckodriver port 4444)
		Cout() << "1. WebDriver Daemon (127.0.0.1:4444): ";
		try {
			HttpRequest req("http://127.0.0.1:4444/status");
			req.Timeout(1500);
			String res = req.Execute();
			if(req.GetStatusCode() == 200) {
				Value st = ParseJSON(res);
				Cout() << "ONLINE (" << AsJSON(st["value"]) << ")\n";
			} else {
				Cout() << "OFFLINE (HTTP " << req.GetStatusCode() << ")\n";
			}
		} catch(const Exc& e) {
			Cout() << "OFFLINE (" << e << ")\n";
		}

		// 2. Active Session Files
		String sess_firefox = GetHomeDirFile(".aria/aria_session_firefox.json");
		Cout() << "2. Session State Files:\n";
		if(FileExists(sess_firefox))
			Cout() << "   - aria_session_firefox.json: " << TrimBoth(LoadFile(sess_firefox)) << "\n";
		else
			Cout() << "   - aria_session_firefox.json: (none)\n";

		// 3. Process Status
		String p_gecko = TrimBoth(Sys("pgrep -a geckodriver"));
		String p_ff = TrimBoth(Sys("pgrep -a firefox | head -n 3"));
		Cout() << "3. Running Processes:\n";
		Cout() << "   geckodriver: " << (p_gecko.IsEmpty() ? "None" : p_gecko) << "\n";
		Cout() << "   firefox:     " << (p_ff.IsEmpty() ? "None" : p_ff) << "\n";

		// 4. Local ManagerSkylark daemon (port 8080)
		Cout() << "4. Local ManagerSkylark (127.0.0.1:8080/api/health): ";
		try {
			HttpRequest req("http://127.0.0.1:8080/api/health");
			req.Timeout(1500);
			String res = req.Execute();
			if(req.GetStatusCode() == 200)
				Cout() << "ONLINE (HTTP 200)\n";
			else
				Cout() << "HTTP " << req.GetStatusCode() << "\n";
		} catch(...) {
			Cout() << "OFFLINE\n";
		}

		// 5. Active Browser Tab Info
		Cout() << "5. Active Browser Tab: ";
		try {
			if(navigator->ConnectToSession("firefox")) {
				String cur_url = navigator->GetCurrentURL();
				Value title_val = navigator->Eval("return document.title;");
				String title = title_val.Is<String>() ? (String)title_val : AsString(title_val);
				Cout() << "\n   URL:   " << cur_url << "\n   Title: " << title << "\n";
			} else {
				Cout() << "Not connected\n";
			}
		} catch(const Exc& e) {
			Cout() << "Error reading active tab: " << e << "\n";
		}
	} else if (cmd == "settings") {
		Cout() << "Show current configuration (not yet implemented).\n";
	} else if (cmd == "tutorial") {
		Cout() << "Start the interactive tutorial (not yet implemented).\n";
	} else if (cmd == "security") {
		Cout() << "Show security best practices (not yet implemented).\n";
	} else if (cmd == "man") {
		cla.PrintHelp();
	} else if (cmd == "help") {
		cla.PrintHelp();
	} else {
		Cout() << "Unknown command: " << cmd << "\n";
		cla.PrintHelp();
	}
}

String Aria::GenerateAIResponse(const String& prompt, const String& context, const String& output_format, const String& provider_name) {
	BaseAIProvider* provider = plugin_manager.GetAIProvider(provider_name);
	if (!provider && provider_name == "gemini") {
		static GeminiProvider default_gemini;
		provider = &default_gemini;
	}
	
	if (!provider) return "Error: AI provider not found: " + provider_name;
	
	return provider->Generate(prompt, context, output_format);
}

bool IsAutomation() {
	if (!GetEnv("UPP_AUTOMATION").IsEmpty()) return true;
	for (const auto& arg : CommandLine()) {
		if (arg == "--test") return true;
	}
	return false;
}

END_UPP_NAMESPACE

#ifdef flagMAIN
CONSOLE_APP_MAIN
{
	bool json_mode = false;
	for (const Upp::String& arg : Upp::CommandLine())
		if (arg == "--json") json_mode = true;
	Upp::StdLogSetup((json_mode ? Upp::LOG_CERR : Upp::LOG_COUT) | Upp::LOG_FILE);
	try {
		Upp::Aria aria;
		aria.Run(Upp::CommandLine());
	} catch (const Upp::Exc& e) {
		if (json_mode)
			Upp::Cout() << Upp::AsJSON(Upp::ValueMap()
				("version", 1)("type", "discord.channel")("error", (Upp::String)e)) << "\n";
		else
			Upp::Cout() << "Error: " << e << "\n";
	} catch (const std::exception& e) {
		if (json_mode)
			Upp::Cout() << Upp::AsJSON(Upp::ValueMap()
				("version", 1)("type", "discord.channel")("error", e.what())) << "\n";
		else
			Upp::Cout() << "System Error: " << e.what() << "\n";
	} catch (...) {
		if (json_mode)
			Upp::Cout() << Upp::AsJSON(Upp::ValueMap()
				("version", 1)("type", "discord.channel")("error", "Unknown error occurred.")) << "\n";
		else
			Upp::Cout() << "Unknown error occurred.\n";
	}
}
#endif
