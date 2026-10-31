#ifndef _WebDriver_Firefox_h_
#define _WebDriver_Firefox_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct FirefoxProfile : public Moveable<FirefoxProfile> { // copyable
	bool accept_untrusted_certs = false;
	bool assume_untrusted_issuer = false;
	String binary;
	bool disable_plugin_whitelist = false;
	bool use_legacy_firefox = false;
	
	void Jsonize(JsonIO& json) {
		json("acceptUntrustedCertificates", accept_untrusted_certs)
			("assumeUntrustedCertificateIssuer", assume_untrusted_issuer)
			("firefox_binary", binary)
			("disablePluginWhitelist", disable_plugin_whitelist)
			("useLegacyFirefox", use_legacy_firefox);
	}
};

struct FirefoxOptions : public Moveable<FirefoxOptions> { // copyable
	Vector<String> args;
	ValueMap prefs;
	ValueMap env;
	String binary;
	String profile;
	Value log;
	
	void Jsonize(JsonIO& json) {
		json("args", args);
		if (json.IsStoring()) {
			if (prefs.GetCount() > 0) json.Set("prefs", prefs);
			if (env.GetCount() > 0) json.Set("env", env);
			if (!binary.IsEmpty()) json.Set("binary", binary);
			if (!profile.IsEmpty()) json.Set("profile", profile);
			if (!log.IsVoid()) json.Set("log", log);
		} else {
			prefs = json.Get("prefs");
			env = json.Get("env");
			binary = (String)json.Get("binary");
			profile = (String)json.Get("profile");
			log = json.Get("log");
		}
	}
	
	void SetPreference(const String& name, const Value& value) {
		prefs.Set(name, value);
	}
	
	void AddArgument(const String& arg) {
		args.Add(arg);
	}
};

struct Firefox : Capabilities { // copyable
	FirefoxProfile profile;
	FirefoxOptions options;
	
	Firefox(const Capabilities& defaults = Capabilities())
		: Capabilities(defaults) {
		browser_name = browser::K_FIREFOX;
		version = String();
	}

	void ApplyStealthSettings(bool silence_audio = false);

	void Jsonize(JsonIO& json) {
		Capabilities::Jsonize(json);
		json("moz:firefoxOptions", options);
	}
};

END_UPP_NAMESPACE

#endif