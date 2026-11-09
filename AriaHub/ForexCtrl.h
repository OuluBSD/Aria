#ifndef _AriaHub_ForexCtrl_h_
#define _AriaHub_ForexCtrl_h_

#include "ServiceCtrl.h"
#include <Aria/ForexScraper.h>

NAMESPACE_UPP

class ForexCtrl : public ServiceCtrl {
public:
	typedef ForexCtrl CLASSNAME;
	ForexCtrl();

	virtual void LoadData() override;
	virtual void Scrape() override;
	virtual String GetTitle() override { return "Forex"; }

private:
	ArrayCtrl  calendar_list;
	ArrayCtrl  trades_list;
	ArrayCtrl  rates_list;
	
	TabCtrl    fx_tabs;
};

END_UPP_NAMESPACE

#endif