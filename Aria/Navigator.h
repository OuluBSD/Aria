#ifndef _Aria_Navigator_h_
#define _Aria_Navigator_h_

#include "BaseAIProvider.h"

class BaseNavigator {
public:
	virtual ~BaseNavigator() {}
	virtual void StartSession(const String& browser_name = "firefox", bool headless = false, bool use_profile = true, const String& profile_path = "") = 0;
	virtual bool ConnectToSession(const String& browser_name = "firefox") = 0;
	virtual void CloseSession(const String& browser_name = "") = 0;
	virtual void Navigate(const String& url) = 0;
	virtual ValueArray ListTabs() = 0;
	virtual bool GotoTab(const Value& identifier) = 0;
	virtual String GetPageContent() = 0;
        virtual String GetCurrentURL() = 0;
	virtual void NewTab(const String& url = "about:blank") = 0;
	virtual void NavigateWithPrompt(const String& prompt) = 0;
};

class AriaNavigator : public BaseNavigator {
	One<WebDriver::Web_driver> driver;
	String session_id;
	double throttle_delay;
	bool randomize_delay;
	bool auto_close_session = false;
	BaseAIProvider* ai_provider = nullptr;
	
	void Throttle();
	String GetSessionFilePath(const String& browser_name = "") const;
	void SaveSession(const String& browser_name, const ValueMap& session_data);
	ValueMap LoadSessionData(const String& browser_name) const;
	String GetCurrentBrowser() const;
	String FindDriverExecutable(const String& exe) const;
	bool ProvisionGeckoDriver();
	void EnsureDriverRunning(const String& browser_name, bool existing_session = false);

public:
	AriaNavigator();
	~AriaNavigator();
	
	void SetAIProvider(BaseAIProvider* ai) { ai_provider = ai; }
	void SetAutoCloseSession(bool close) { auto_close_session = close; }
	//Whether the recorded session for 'browser_name' was started headless (no visible window).
	bool SessionIsHeadless(const String& browser_name = "firefox") const;
	
	virtual void StartSession(const String& browser_name = "firefox", bool headless = false, bool use_profile = true, const String& profile_path = "") override;
	virtual bool ConnectToSession(const String& browser_name = "firefox") override;
	virtual void CloseSession(const String& browser_name = "") override;
	virtual void Navigate(const String& url) override;
	virtual ValueArray ListTabs() override;
	virtual bool GotoTab(const Value& identifier) override;
	virtual String GetPageContent() override;
        virtual String GetCurrentURL() override;
	virtual void NewTab(const String& url = "about:blank") override;
	virtual void NavigateWithPrompt(const String& prompt) override;
	
	Element WaitForElement(const String& selector, const String& by = "css selector", int timeout = 10);
	Vector<String> GetTabsByTag(const String& tag);
	void TagTab(const Value& identifier, const String& tag);
	ValueArray ExtractLinks();
	Value Eval(const String& script);
	
	Element FindElement(const By& by);
	Vector<Element> FindElements(const By& by);
};

#endif
