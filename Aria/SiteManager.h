#ifndef _Aria_SiteManager_h_
#define _Aria_SiteManager_h_

class SiteManager {
	String base_dir;
public:
	SiteManager(const String& base_dir = "");
	
	String GetSiteDir(const String& site_name) const;
	void SaveData(const String& site_name, const String& filename, const Value& data);
	Value LoadData(const String& site_name, const String& filename) const;
	ValueArray GetRecentItems(const String& site_name, const String& filename, const String& key = "items", int limit = 5) const;
	Vector<String> ListSites() const;
	ValueMap GetRegistry(const String& site_name) const;
	void UpdateRegistry(const String& site_name, const Vector<String>& item_names);
	void SetSiteData(const String& site_name, const String& key, const Value& data);
	Value GetSiteData(const String& site_name, const String& key) const;
};

#endif
