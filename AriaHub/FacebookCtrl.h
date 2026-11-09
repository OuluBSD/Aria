#ifndef _AriaHub_FacebookCtrl_h_
#define _AriaHub_FacebookCtrl_h_

#include "ServiceCtrl.h"
#include <Aria/FacebookScraper.h>

NAMESPACE_UPP

class FacebookCtrl : public ServiceCtrl {
public:
	typedef FacebookCtrl CLASSNAME;
	FacebookCtrl();

	virtual void LoadData() override;
	virtual void Scrape() override;
	virtual String GetTitle() override { return "Facebook"; }

private:
	ArrayCtrl  feed_list;
	ArrayCtrl  friends_list;
	
	TabCtrl    fb_tabs;
};

END_UPP_NAMESPACE

#endif
