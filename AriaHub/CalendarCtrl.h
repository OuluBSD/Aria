#ifndef _AriaHub_CalendarCtrl_h_
#define _AriaHub_CalendarCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class CalendarCtrl : public ServiceCtrl {
public:
	typedef CalendarCtrl CLASSNAME;
	CalendarCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual void   RefreshSubTab(int tab_index) override { Scrape(); }
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "Calendar"; }
	virtual Image  GetIcon() override  { return Image(); }
	virtual int    GetActiveTab() override { return 0; }

private:
	ArrayCtrl list; // Using ArrayCtrl for now as TreeArrayCtrl might be complex to wire quickly
	Button    btnRefresh;
};

END_UPP_NAMESPACE

#endif
