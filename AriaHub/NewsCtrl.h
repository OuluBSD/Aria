#ifndef _AriaHub_NewsCtrl_h_
#define _AriaHub_NewsCtrl_h_

#include "ServiceCtrl.h"
#include <Aria/NewsScraper.h>

NAMESPACE_UPP

class NewsCtrl : public ServiceCtrl {
public:
	typedef NewsCtrl CLASSNAME;
	NewsCtrl();
	virtual ~NewsCtrl();
	
	virtual void   LoadData() override;
	virtual void   Scrape() override;
	virtual String GetTitle() override { return "News"; }
	virtual Image  GetIcon() override  { return Image(); }
	
	void Store();
	void Load();
    String GetStorePath() const { return ConfigFile("News.bin"); }

private:
	ArrayCtrl list;
	Button    btnRefresh;
	
	VectorMap<String, NewsItem> items; // Keyed by URL/ID
	
	Thread work_thread;
	bool   is_working = false;
	
	void ScrapeProvider(const String& name, const String& url);
};

END_UPP_NAMESPACE

#endif
