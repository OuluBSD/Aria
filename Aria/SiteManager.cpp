#include "Aria.h"

NAMESPACE_UPP

SiteManager::SiteManager(const String& base_dir) {
	if (base_dir.IsEmpty()) {
		this->base_dir = GetHomeDirFile(AppendFileName(".aria", "sites"));
	} else {
		this->base_dir = base_dir;
	}
	RealizeDirectory(this->base_dir);
}

String SiteManager::GetSiteDir(const String& site_name) const {
	String site_dir = AppendFileName(base_dir, site_name);
	RealizeDirectory(site_dir);
	RealizeDirectory(AppendFileName(site_dir, "media"));
	return site_dir;
}

void SiteManager::SaveData(const String& site_name, const String& filename, const Value& data) {
	String path = AppendFileName(GetSiteDir(site_name), filename);
	if (!SaveFile(path, AsJSON(data, true))) {
		GetAriaLogger("site_manager").Error("Failed to save data to " + path);
	}
}

Value SiteManager::LoadData(const String& site_name, const String& filename) const {
	String path = AppendFileName(AppendFileName(base_dir, site_name), filename);
	if (!FileExists(path)) return Value();
	return ParseJSON(LoadFile(path));
}

ValueArray SiteManager::GetRecentItems(const String& site_name, const String& filename, const String& key, int limit) const {
	Value data = LoadData(site_name, filename);
	if (data.IsVoid()) return ValueArray();
	
	ValueArray items;
	if (data.Is<ValueArray>()) items = data;
	else if (data.Is<ValueMap>()) items = data[key];
	
	if (items.IsEmpty()) return ValueArray();
	
	// Simplified recent items: return last N
	ValueArray res;
	for (int i = max(0, items.GetCount() - limit); i < items.GetCount(); i++) {
		res.Add(items[i]);
	}
	return res;
}

Vector<String> SiteManager::ListSites() const {
	Vector<String> res;
	FindFile ff(AppendFileName(base_dir, "*"));
	while (ff) {
		if (ff.IsFolder()) res.Add(ff.GetName());
		ff.Next();
	}
	return res;
}

ValueMap SiteManager::GetRegistry(const String& site_name) const {
	Value reg = LoadData(site_name, "registry.json");
	if (reg.Is<ValueMap>()) return reg;
	return ValueMap()("next_id", 1)("mappings", ValueMap());
}

void SiteManager::UpdateRegistry(const String& site_name, const Vector<String>& item_names) {
	ValueMap reg = GetRegistry(site_name);
	ValueMap mappings = reg["mappings"];
	int next_id = reg["next_id"];
	
	ValueMap name_to_id;
	for (int i = 0; i < mappings.GetCount(); i++) {
		name_to_id.Set(mappings.GetValue(i), mappings.GetKey(i));
	}
	
	bool changed = false;
	for (const String& name : item_names) {
		if (name_to_id.Find(name) < 0) {
			String id = AsString(next_id++);
			mappings.Set(id, name);
			changed = true;
		}
	}
	
	if (changed) {
		reg.Set("mappings", mappings);
		reg.Set("next_id", next_id);
		SaveData(site_name, "registry.json", reg);
	}
}

void SiteManager::SetSiteData(const String& site_name, const String& key, const Value& data) {
	SaveData(site_name, key + ".json", data);
}

Value SiteManager::GetSiteData(const String& site_name, const String& key) const {
	return LoadData(site_name, key + ".json");
}

END_UPP_NAMESPACE
