#ifndef _AriaHub_YouTubeCtrl_h_
#define _AriaHub_YouTubeCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class YouTubeCtrl : public ServiceCtrl {
public:
	typedef YouTubeCtrl CLASSNAME;
	YouTubeCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual void   RefreshSubTab(int tab_index) override { Scrape(); }
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "YouTube"; }
	virtual Image  GetIcon() override  { return Image(); }

private:
	TabCtrl   tabs;
	ArrayCtrl feedList;
	ArrayCtrl studioList;
	ArrayCtrl commentList;
	Button    btnRefresh;
	
	void LoadFeed();
	void LoadStudio();
	void LoadComments();
};

END_UPP_NAMESPACE

#endif