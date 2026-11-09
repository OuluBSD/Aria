#ifndef _AriaHub_WhatsAppCtrl_h_
#define _AriaHub_WhatsAppCtrl_h_

#include "ServiceCtrl.h"

NAMESPACE_UPP

class WhatsAppCtrl : public ServiceCtrl {
public:
	typedef WhatsAppCtrl CLASSNAME;
	WhatsAppCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual String GetTitle() override { return "WhatsApp"; }
	virtual Image  GetIcon() override  { return CtrlImg::plus(); }

private:
	ArrayCtrl list;
	Button btnRefresh;
};

END_UPP_NAMESPACE

#endif
