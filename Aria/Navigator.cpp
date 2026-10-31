#include "Aria.h"

NAMESPACE_UPP

AriaNavigator::AriaNavigator() {
	String td = GetEnv("ARIA_THROTTLE_DELAY");
	throttle_delay = td.IsEmpty() ? 1.0 : ScanDouble(td);
	randomize_delay = ToLower(GetEnv("ARIA_RANDOMIZE_DELAY")) != "false";
}

AriaNavigator::~AriaNavigator() {
	try {
		if (driver && auto_close_session) {
			GetAriaLogger("navigator").Info("AriaNavigator: Auto-closing session...");
			driver->DeleteSession();
		}
	} catch(...) {}
}

void AriaNavigator::Throttle() {
	if (throttle_delay > 0) {
		double delay = throttle_delay;
		if (randomize_delay) {
			delay = delay * (0.8 + (Random(400) / 1000.0));
		}
		GetAriaLogger("navigator").Info(Format("Throttling for %.2f s", delay));
		Sleep(int(delay * 1000));
	}
}

String AriaNavigator::GetSessionFilePath(const String& browser_name) const {
	String aria_dir = GetHomeDirFile(".aria");
	RealizeDirectory(aria_dir);
	if (!browser_name.IsEmpty())
		return AppendFileName(aria_dir, "aria_session_" + browser_name + ".json");
	return AppendFileName(aria_dir, "aria_session_current.json");
}

void AriaNavigator::SaveSession(const String& browser_name, const ValueMap& session_data) {
	SaveFile(GetSessionFilePath(browser_name), AsJSON(session_data));
}

ValueMap AriaNavigator::LoadSessionData(const String& browser_name) const {
	String data = LoadFile(GetSessionFilePath(browser_name));
	if (data.IsEmpty()) return ValueMap();
	return ParseJSON(data);
}

String AriaNavigator::GetCurrentBrowser() const {
	ValueMap data = LoadSessionData("");
	return data["browser"];
}

bool AriaNavigator::SessionIsHeadless(const String& browser_name) const {
	ValueMap data = LoadSessionData(browser_name);
	Value h = data["headless"];
	//A record without the flag predates headless tracking; we cannot prove a window is
	//visible, so report it headless and let 'open' start a fresh, visible browser once.
	return !h.Is<bool>() || (bool)h;
}

//Look for a WebDriver executable on PATH first, then in Aria's own
//provisioning directory (~/.aria/bin), where self-downloaded drivers live.
String AriaNavigator::FindDriverExecutable(const String& exe) const {
	String path = GetFileOnPath(exe, GetEnv("PATH"));
	if (!path.IsEmpty())
		return path;
	String local = GetHomeDirFile(".aria/bin/" + exe);
	return FileExists(local) ? local : String();
}

//Download the latest geckodriver release from mozilla into ~/.aria/bin so a
//fresh system can run Firefox sessions without a package manager install.
bool AriaNavigator::ProvisionGeckoDriver() {
	String dir = GetHomeDirFile(".aria/bin");
	RealizeDirectory(dir);
	String exe_path = AppendFileName(dir, "geckodriver");
	if (FileExists(exe_path))
		return true;

	String arch = TrimBoth(Sys("sh", {"-c", "uname -m"}));
	String platform = arch == "aarch64" ? "linux-aarch64" : "linux64";
	String tmp = AppendFileName(dir, "geckodriver.tar.gz");
	String script =
		"set -e\n"
		"url=$(curl -fsSL https://api.github.com/repos/mozilla/geckodriver/releases/latest"
		" | grep -oE 'https://[^\\\"]*geckodriver-v[^\\\"]*-" + platform + ".tar.gz' | head -n 1)\n"
		"test -n \"$url\"\n"
		"curl -fsSL -o \"" + tmp + "\" \"$url\"\n"
		"tar -xzf \"" + tmp + "\" -C \"" + dir + "\"\n"
		"chmod +x \"" + exe_path + "\"\n"
		"rm -f \"" + tmp + "\"\n";
	Sys("sh", {"-c", script});

	if (!FileExists(exe_path)) {
		GetAriaLogger("navigator").Error("Failed to download geckodriver into " + dir + ".");
		return false;
	}
	GetAriaLogger("navigator").Info("Downloaded geckodriver to " + exe_path + ".");
	return true;
}

void AriaNavigator::EnsureDriverRunning(const String& browser_name, bool existing_session) {
	int port = 4444;
	String exe = browser_name == "firefox" ? "geckodriver" : "chromedriver";

	try {
		HttpRequest req("http://127.0.0.1:4444/status");
		req.Timeout(1000);
		String res = req.Execute();
		if (existing_session || res.Find("\"ready\":true") >= 0) {
			GetAriaLogger("navigator").Info("WebDriver (" + exe + ") already running and responding.");
			return;
		}
	} catch(...) {}

	GetAriaLogger("navigator").Info("WebDriver (" + exe + ") not responding or not detected. Starting...");

	if (Sys("pgrep " + exe).GetCount() > 0) {
		GetAriaLogger("navigator").Warning("Cleaning up unresponsive " + exe);
		system("killall -9 " + exe + " 2>/dev/null");
		Sleep(500);
	}

	String path = FindDriverExecutable(exe);
	if (path.IsEmpty() && browser_name == "firefox") {
		ProvisionGeckoDriver();
		path = FindDriverExecutable(exe);
	}
	if (path.IsEmpty()) {
		GetAriaLogger("navigator").Error("Could not find " + exe + " in PATH.");
		throw SessionError("Could not find " + exe + " on this system. Install it (e.g. 'sudo emerge net-misc/geckodriver' on Gentoo) or let Aria provision it into ~/.aria/bin/.");
	}

	String cmd;
	if (browser_name == "firefox") {
		cmd = path + " --host 127.0.0.1 --port " + AsString(port);
	} else if (browser_name == "chrome") {
		cmd = path + " --port " + AsString(port);
	} else {
		return;
	}
	
	String log_file = GetHomeDirFile(".aria/" + exe + ".log");
	system("nohup " + cmd + " > " + log_file + " 2>&1 &");
	
	for (int i = 0; i < 20; i++) {
		Sleep(500);
		try {
			HttpRequest req("http://127.0.0.1:4444/status");
			req.Timeout(500);
			if (!req.Execute().IsEmpty()) {
				GetAriaLogger("navigator").Info(exe + " started successfully.");
				return;
			}
		} catch(...) {}
	}
	
	GetAriaLogger("navigator").Error("Failed to start " + exe + ".");
}

void AriaNavigator::StartSession(const String& browser_name, bool headless, bool use_profile, const String& profile_path) {
	if (driver) {
		try {
			driver->GetUrl();
			return; 
		} catch(...) {
			driver.Clear();
		}
	}
	// Upp::Sys() runs the command directly via execve with no shell, so a pipe
	// here is passed to ps as a literal argument and ps errors out -> empty
	// result -> the guard silently never fires. Route the pipe through sh -c.
	if (browser_name == "firefox" && use_profile &&
	    !TrimBoth(Sys("sh", {"-c", "ps -eo stat=,comm= | awk '$1 !~ /^Z/ && ($2 == \"firefox\" || $2 == \"firefox-bin\") {print}'"})).IsEmpty())
		throw SessionError("Close all Firefox instances before starting the Aria Firefox session with user profile.");

	GetAriaLogger("navigator").Info("Starting fresh browser session: " + browser_name + (headless ? " (headless)" : ""));
	
	EnsureDriverRunning(browser_name);
	
	try {
		if (browser_name == "firefox") {
			Firefox caps;
			caps.ApplyStealthSettings();
			String patched_firefox = detail::PatchFirefoxBinary();
			if (patched_firefox.IsEmpty())
				throw SessionError("Could not prepare patched Firefox binary.");
			caps.options.binary = patched_firefox;
			if (use_profile) {
				String profile = profile_path.IsEmpty() ? detail::GetFirefoxDefaultProfilePath() : profile_path;
				if (!profile.IsEmpty() && DirectoryExists(profile)) {
					GetAriaLogger("navigator").Info("Using Firefox profile: " + profile);
					caps.options.AddArgument("-profile");
					caps.options.AddArgument(profile);
				} else if (!profile.IsEmpty()) {
					GetAriaLogger("navigator").Error("Profile directory does not exist: " + profile);
				} else {
					GetAriaLogger("navigator").Warning("Could not detect default Firefox profile; starting without profile.");
				}
			} else {
				GetAriaLogger("navigator").Info("Starting without user profile (--no-profile specified).");
			}
			if (headless) caps.options.AddArgument("--headless");
			
			driver = new WebDriver::Web_driver(caps, K_DEFAULT_WEB_DRIVER_URL,
				auto_close_session ? detail::Resource::IS_OWNER : detail::Resource::IS_OBSERVER);
			session_id = driver->GetSessionId();
			
			ValueMap sd;
			sd.Add("session_id", session_id);
			sd.Add("browser", browser_name);
			sd.Add("headless", headless);
			SaveSession(browser_name, sd);
			SaveSession("", sd); 
			
			GetAriaLogger("navigator").Info("Session started: " + session_id);
		}
	} catch (const std::exception& e) {
		GetAriaLogger("navigator").Error("Failed to start session: " + String(e.what()));
		throw;
	}
}

bool AriaNavigator::ConnectToSession(const String& browser_name) {
	ValueMap session_data = LoadSessionData(browser_name);
	String sid = session_data["session_id"];
	
	if (sid.IsEmpty()) return false;
	
	GetAriaLogger("navigator").Info("Attempting to connect to existing session: " + sid);
	
	EnsureDriverRunning(browser_name, true);
	
	try {
		driver = new WebDriver::Web_driver(sid);
		try {
			driver->GetUrl(); // Test connection
		} catch (...) {
			auto windows = driver->GetWindows();
			if (!windows.IsEmpty()) {
				driver->SetFocusToWindow(windows[0]);
				driver->GetUrl();
			} else {
				throw;
			}
		}
		session_id = sid;
		auto_close_session = false;
		GetAriaLogger("navigator").Info("Successfully reconnected to " + browser_name + " session " + sid);
		return true;
	} catch (...) {
		GetAriaLogger("navigator").Warning("Failed to reconnect to session " + sid + ". It may have expired.");
		driver.Clear();
		return false;
	}
}

void AriaNavigator::CloseSession(const String& browser_name) {
	if (driver) {
		try {
			driver->DeleteSession();
		} catch(...) {}
		driver.Clear();
	}
	String path = GetSessionFilePath(browser_name);
	if (FileExists(path)) FileDelete(path);
	if (browser_name.IsEmpty()) { // If we closed the current one, clear current mapping
		path = GetSessionFilePath("");
		if (FileExists(path)) FileDelete(path);
	}
}

void AriaNavigator::Navigate(const String& url) {
	Throttle();
	if (!driver && !ConnectToSession("firefox")) { 
		StartSession("firefox", IsAutomation());
		if (!driver) throw SessionError("Could not connect to or start a session.");
	}
	GetAriaLogger("navigator").Info("Navigating to: " + url);
	try {
		driver->ApplyStealthJS();
		driver->Get(url);
		driver->ApplyStealthJS();
	} catch (const std::exception& e) {
		GetAriaLogger("navigator").Error("Navigation failed: " + String(e.what()));
		if (ConnectToSession("firefox")) {
			driver->ApplyStealthJS();
			driver->Get(url);
			driver->ApplyStealthJS();
		} else {
			throw;
		}
	}
}

ValueArray AriaNavigator::ListTabs() {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	ValueArray tabs;
	Vector<Window> windows = driver->GetWindows();
	Window current = driver->GetCurrentWindow();
	
	for (int i = 0; i < windows.GetCount(); i++) {
		Window& w = windows[i];
		driver->SetFocusToWindow(w);
		tabs.Add(ValueMap()
			("index", i)
			("handle", w.GetHandle())
			("title", driver->GetTitle())
			("url", driver->GetUrl())
			("active", w.GetHandle() == current.GetHandle())
		);
	}
	
	driver->SetFocusToWindow(current);
	return tabs;
}

bool AriaNavigator::GotoTab(const Value& identifier) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	Vector<Window> windows = driver->GetWindows();
	if (identifier.Is<int>()) {
		int idx = identifier;
		if (idx >= 0 && idx < windows.GetCount()) {
			driver->SetFocusToWindow(windows[idx]);
			return true;
		}
	} else if (identifier.Is<String>()) {
		String target = identifier;
		for (const auto& w : windows) {
			driver->SetFocusToWindow(w);
			if (driver->GetTitle().Find(target) >= 0 || driver->GetUrl().Find(target) >= 0 || w.GetHandle() == target)
				return true;
		}
	}
	return false;
}

String AriaNavigator::GetPageContent() {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->GetSource();
}

void AriaNavigator::NewTab(const String& url) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	driver->Execute("window.open('" + url + "', '_blank');", JsArgs());
}

void AriaNavigator::NavigateWithPrompt(const String& prompt) {
	// AI-driven navigation logic
}

Element AriaNavigator::WaitForElement(const String& selector, const String& by, int timeout) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	
	TimeStop ts;
	while (ts.Elapsed() < timeout * 1000) {
		try {
			Element e = driver->FindElement(By(by, selector));
			if (e.IsDisplayed()) return e;
		} catch (...) {}
		Sleep(500);
	}
	
	throw BrowserError("Timed out waiting for element: " + selector);
}

Vector<String> AriaNavigator::GetTabsByTag(const String& tag) {
	return Vector<String>();
}

void AriaNavigator::TagTab(const Value& identifier, const String& tag) {
}

Value AriaNavigator::Eval(const String& script) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->Eval<Value>(script);
}

Element AriaNavigator::FindElement(const By& by) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->FindElement(by);
}

Vector<Element> AriaNavigator::FindElements(const By& by) {
	if (!driver && !ConnectToSession()) throw SessionError("No active session.");
	return driver->FindElements(by);
}


String AriaNavigator::GetCurrentURL() {
        if (!driver && !ConnectToSession()) throw SessionError("No active session.");
        return driver->GetUrl();
}

END_UPP_NAMESPACE
