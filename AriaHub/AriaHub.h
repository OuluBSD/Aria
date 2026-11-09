#ifndef _AriaHub_AriaHub_h_
#define _AriaHub_AriaHub_h_

#include <Aria/Aria.h>
#include <CtrlLib/CtrlLib.h>
#include <Ctrl/Automation/Automation.h>
#include <ByteVM/ByteVM.h>
#include "ThreadsCtrl.h"
#include "NewsCtrl.h"
#include "ForexCtrl.h"
#include "FacebookCtrl.h"
#include "WhatsAppCtrl.h"
#include "GoogleMessagesCtrl.h"
#include "UniversalInboxCtrl.h"
#include "YouTubeCtrl.h"
#include "CalendarCtrl.h"

NAMESPACE_UPP

class AriaMainWindow : public TopWindow {
public:
	typedef AriaMainWindow CLASSNAME;
	AriaMainWindow();

	virtual void MainMenu(Bar& bar);
	virtual void ServiceMenu(Bar& bar);

	void RefreshActiveSubTab();
	void RefreshActiveService();
	void RefreshAllServices();

	void RefreshAll();
	void OpenSettings();
	void StopScrapers();

	virtual bool Key(dword key, int count) override;

	AriaNavigator& GetNavigator() { return aria.GetNavigator(); }

private:
	MenuBar   menu;
	TabCtrl   tabs;
	ToolBar   toolbar;
	StatusBar statusbar;
	
	Aria      aria; // Backend integration
	
	ThreadsCtrl        threads;
		NewsCtrl news;
			ForexCtrl    forex;
			FacebookCtrl facebook;
			WhatsAppCtrl whatsapp;
		
	
	GoogleMessagesCtrl google_messages;
	UniversalInboxCtrl universal_inbox;
	YouTubeCtrl        youtube;
	CalendarCtrl       calendar;
};

void AriaAlert(const String& msg);
bool IsAutomation();

END_UPP_NAMESPACE

#endif