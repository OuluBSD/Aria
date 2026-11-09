#ifndef _AriaHub_ServiceCtrl_h_
#define _AriaHub_ServiceCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <Aria/Aria.h>

NAMESPACE_UPP

class ServiceCtrl : public ParentCtrl {
public:
	typedef ServiceCtrl CLASSNAME;
	ServiceCtrl() : navigator(nullptr), site_manager(nullptr) {}
	
	virtual void   SetNavigator(AriaNavigator* nav, SiteManager* sm) { navigator = nav; site_manager = sm; }
	virtual void   LoadData() = 0;
	virtual void   Scrape() = 0;
	virtual void   RefreshSubTab(int tab_index) { Scrape(); } // Default to full scrape if not specialized
	virtual void   RefreshService() { Scrape(); }
	
	virtual String GetTitle() = 0;
	virtual Image  GetIcon() { return Image(); }
	virtual int    GetActiveTab() { return -1; }

	Event<String> WhenStatus;

protected:
	AriaNavigator* navigator;
	SiteManager*   site_manager;
};

END_UPP_NAMESPACE

#endif
