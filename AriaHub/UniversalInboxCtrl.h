#ifndef _AriaHub_UniversalInboxCtrl_h_
#define _AriaHub_UniversalInboxCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class UniversalInboxCtrl : public ServiceCtrl {
public:
	typedef UniversalInboxCtrl CLASSNAME;
	UniversalInboxCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override; // Refreshes all services
	virtual void   RefreshSubTab(int tab_index) override { Scrape(); }
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "Universal Inbox"; }
	virtual Image  GetIcon() override  { return Image(); }
	virtual int    GetActiveTab() override { return 0; }
	
	Event<int, int> WhenJump; // service_idx, sub_idx

private:
	ArrayCtrl list;
	Button    btnRefresh;
	
	void OnDouble();
};

END_UPP_NAMESPACE

#endif
