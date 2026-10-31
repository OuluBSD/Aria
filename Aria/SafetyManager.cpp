#include "Aria.h"

NAMESPACE_UPP

SafetyManager::SafetyManager() {}

bool SafetyManager::CheckUrlSafety(const String& url, bool force) {
	if (force) return true;
	
	static const char* sensitive_keywords[] = {
		"login", "password", "bank", "account", "checkout", "payment", "crypto", "wallet"
	};
	
	String lower_url = ToLower(url);
	for (const char* k : sensitive_keywords) {
		if (lower_url.Find(k) >= 0) {
			GetAriaLogger("safety_manager").Warning("Sensitive keyword found in URL: " + String(k));
			if (GetEnv("ARIA_NON_INTERACTIVE") == "true") return false;
			return Confirm("The URL contains a sensitive keyword ('" + String(k) + "'). Proceed?");
		}
	}
	return true;
}

void SafetyManager::EnsureDisclaimerAccepted() {
	String aria_dir = GetHomeDirFile(".aria");
	String disclaimer_file = AppendFileName(aria_dir, "disclaimer_accepted");
	
	if (FileExists(disclaimer_file)) return;
	
	if (GetEnv("ARIA_NON_INTERACTIVE") == "true") {
		GetAriaLogger("safety_manager").Error("Disclaimer not accepted and running in non-interactive mode.");
		throw AriaError("Legal disclaimer must be accepted. Create " + disclaimer_file + " to bypass.");
	}
	
	Cout() << "\n--- ARIA LEGAL DISCLAIMER ---\n"
	       << "Aria is an automated browser tool. By using it, you agree that:\n"
	       << "1. You are responsible for all actions taken by the tool.\n"
	       << "2. You will not use it for malicious or illegal purposes.\n"
	       << "3. You understand that AI responses may be inaccurate.\n";
	
	if (Confirm("Do you accept these terms?")) {
		SaveFile(disclaimer_file, AsString(GetUtcTime()));
	} else {
		throw AriaError("Disclaimer rejected.");
	}
}

bool SafetyManager::Confirm(const String& message) {
	Cout() << message << " (Proceeding automatically in 2s...)\n";
	Sleep(2000);
	return true;
}

String SafetyManager::GetSecurityBestPractices() {
	return "Security Best Practices:\n"
	       "- Never share your credentials.json file.\n"
	       "- Use unique passwords for different sites.\n"
	       "- Review scripts before running them.\n"
	       "- Use --force only in trusted environments.";
}

END_UPP_NAMESPACE
