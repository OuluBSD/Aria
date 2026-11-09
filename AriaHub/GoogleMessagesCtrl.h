#ifndef _AriaHub_GoogleMessagesCtrl_h_
#define _AriaHub_GoogleMessagesCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class GoogleMessagesCtrl : public ServiceCtrl {
public:
	typedef GoogleMessagesCtrl CLASSNAME;
	GoogleMessagesCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual void   RefreshSubTab(int tab_index) override { Scrape(); }
	virtual void   RefreshService() override { Scrape(); }
	virtual String GetTitle() override { return "Google Messages"; }
	virtual Image  GetIcon() override  { return Image(); }
	virtual int    GetActiveTab() override { return 0; }

private:
	ArrayCtrl list;
	Button    btnRefresh;
	EditString search;
	Label      lblSearch;
	
	// OTP Banner
	ParentCtrl otp_banner;
	Label      lblOtp;
	Button     btnCopyOtp;
	Option     auto_copy_otp;
	
	String     last_otp;
	
	void OnCopyOtp();
};

END_UPP_NAMESPACE

#endif
