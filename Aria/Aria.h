#ifndef _Aria_Aria_h_
#define _Aria_Aria_h_

#include <Core/Core.h>
#include <WebDriver/WebDriver.h>

NAMESPACE_UPP

#include "Exceptions.h"
#include "Logger.h"
#include "Utils.h"
#include "CredentialManager.h"
#include "Navigator.h"
#include "ScriptManager.h"
#include "SafetyManager.h"
#include "ReportManager.h"
#include "SiteManager.h"
#include "PluginManager.h"
#include "UndetectedFirefox.h"
#include "UndetectedAria.h"
#include "GoogleMessagesScraper.h"
#include "WhatsAppScraper.h"
#include "DiscordScraper.h"
#include "SlackScraper.h"
#include "ThreadsScraper.h"
#include "NewsScraper.h"
#include "CalendarScraper.h"
#include "YouTubeStudioScraper.h"
#include "YouTube.h"
#include "YouTubeScraper.h"
#include "Forex.h"
#include "ForexScraper.h"
#include "Facebook.h"
#include "FacebookScraper.h"
#include "SunoScraper.h"
#include "TyomarkkinatoriScraper.h"
#include "DuunitoriScraper.h"
#include "ManagerSkylarkScraper.h"
#include "GitHubScraper.h"
#include "SourceForgeScraper.h"

class GeminiProvider : public BaseAIProvider {
public:
	virtual String Generate(const String& prompt, const String& context = "", const String& output_format = "text") override;
};

class OtpManager {
public:
	static String ExtractOtp(const String& text);
};

class Aria {
	ScriptManager script_manager;
	SafetyManager safety_manager;
	ReportManager report_manager;
	SiteManager site_manager;
	PluginManager plugin_manager;
	One<AriaNavigator> navigator;
	
public:
	Aria();
	
	void Run(const Vector<String>& args);
	
	String GenerateAIResponse(const String& prompt, const String& context = "", const String& output_format = "text", const String& provider_name = "gemini");
	
	AriaNavigator& GetNavigator() { return *navigator; }
	SiteManager& GetSiteManager() { return site_manager; }
};

bool IsAutomation();

END_UPP_NAMESPACE

#endif
