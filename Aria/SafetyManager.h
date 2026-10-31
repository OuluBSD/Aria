#ifndef _Aria_SafetyManager_h_
#define _Aria_SafetyManager_h_

class SafetyManager {
public:
	SafetyManager();
	
	bool CheckUrlSafety(const String& url, bool force = false);
	void EnsureDisclaimerAccepted();
	bool Confirm(const String& message);
	String GetSecurityBestPractices();
};

#endif
