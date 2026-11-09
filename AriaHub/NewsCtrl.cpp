#include "NewsCtrl.h"
#include "AriaHub.h"

NAMESPACE_UPP

NewsCtrl::NewsCtrl() {
	Load();
	
	Add(list.SizePos());
	list.AddColumn("Time").SetConvert(StdConvert());
	list.AddColumn("Source");
	list.AddColumn("Title", 4);
	list.AddColumn("Summary", 2);
	
	list.LayoutId("NewsList");
    list.ColumnWidths("2 2 8 4");
    
    Add(btnRefresh.RightPos(10, 100).BottomPos(10, 30));
    btnRefresh.SetLabel("Refresh");
    btnRefresh.LayoutId("BtnRefresh");
    btnRefresh << [=] { Scrape(); };
    
    LayoutId("NewsCtrl");
	LoadData();
}

NewsCtrl::~NewsCtrl() {
	if (work_thread.IsOpen()) {
        is_working = false;
        work_thread.Wait();
    }
	Store();
}

void NewsCtrl::Store() {
    if(!Thread::IsMain()) return;
	StoreToFile(*this, GetStorePath());
}

void NewsCtrl::Load() {
    if(!Thread::IsMain()) return;
	LoadFromFile(*this, GetStorePath());
}

void NewsCtrl::LoadData() {
    if(!Thread::IsMain()) return;
	list.Clear();
    
    // Sort by time desc
    Vector<String> keys;
    for(int i = 0; i < items.GetCount(); i++)
        keys.Add(items.GetKey(i));

    Sort(keys, [&](const String& a, const String& b) {
        return items.Get(a).published > items.Get(b).published;
    });
    
    for(const String& k : keys) {
        const NewsItem& n = items.Get(k);
        list.Add(n.published, n.source, n.title, n.summary);
    }
}

void NewsCtrl::Scrape() {
	if (is_working || !navigator || !site_manager) {
		Cout() << "Scrape already working or navigator missing\n";
		return;
	}
	is_working = true;
	Cout() << "Scrape() triggered, starting work_thread\n";
	WhenStatus("Scraping News...");
	
	work_thread.Run([=] {
		Cout() << "News work_thread started\n";
        const struct { const char* name; const char* url; } providers[] = {
            { "ZeroHedge", "https://www.zerohedge.com/" },
            { "NaturalNews", "https://www.naturalnews.com/" },
            { "GlobalResearch", "https://www.globalresearch.ca/" },
            { "HackerNews", "https://news.ycombinator.com/" },
            { "ForexFactory", "https://www.forexfactory.com/" },
            { "FXStreet", "https://www.fxstreet.com" },
            { "Investing", "https://www.investing.com" },
            { "Discord", "https://discord.com/channels/283274943754665984/1363100565890007041" },
            { "Discord", "https://discord.com/channels/283274943754665984/675830944371965953" }
        };
        
        for(const auto& p : providers) {
            Cout() << "Processing provider: " << p.name << "\n";
            ScrapeProvider(p.name, p.url);
            Sleep(5000); 
            if(!is_working) {
				Cout() << "Scrape aborted\n";
				break;
            }
        }
        
		PostCallback([=] {
			is_working = false;
			Cout() << "Scrape finished\n";
			WhenStatus("Ready");
            LoadData(); 
		});
	});
}

void NewsCtrl::ScrapeProvider(const String& name, const String& url) {
    if(!navigator || !site_manager) return;
    
    Cout() << "Scraping site: " << name << " url: " << url << "\n";
    NewsScraper scraper(*navigator, *site_manager);
    ValueArray res = scraper.ScrapeSite(name, url);
    Cout() << "Scraper returned " << res.GetCount() << " items for " << name << "\n";
    
    PostCallback([=] {
        bool changed = false;
        for(const Value& v : res) {
            String u = v["url"];
            if(u.IsEmpty()) continue;
            
            if(items.Find(u) < 0) {
                NewsItem& n = items.Add(u);
                n.url = u;
                n.title = v["title"];
                n.source = v["source"];
                n.summary = v["summary"];
                n.published = GetUtcTime(); 
                n.scraped_time = GetUtcTime();
                n.id = u;
                changed = true;
            }
        }
        if(changed) {
            Store();
            LoadData();
        }
        WhenStatus("Scraped " + name);
    });
}

END_UPP_NAMESPACE