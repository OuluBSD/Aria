#include "Aria.h"

NAMESPACE_UPP

ScriptManager::ScriptManager() {
	scripts_dir = GetHomeDirFile(AppendFileName(".aria", "scripts"));
	metadata_file = AppendFileName(scripts_dir, "metadata.json");
	
	RealizeDirectory(scripts_dir);
	
	if (!FileExists(metadata_file)) {
		SaveMetadata(ValueMap()("scripts", ValueArray()));
	}
}

ValueMap ScriptManager::LoadMetadata() const {
	if (!FileExists(metadata_file)) return ValueMap()("scripts", ValueArray());
	Value v = ParseJSON(LoadFile(metadata_file));
	if (v.Is<ValueMap>()) return v;
	return ValueMap()("scripts", ValueArray());
}

void ScriptManager::SaveMetadata(const ValueMap& metadata) {
	SaveFile(metadata_file, AsJSON(metadata, true));
}

int ScriptManager::CreateScript(const String& prompt, const String& name) {
	ValueMap metadata = LoadMetadata();
	ValueArray scripts = metadata["scripts"];
	int script_id = scripts.GetCount();
	
	ScriptEntry s;
	s.id = script_id;
	s.name = name.IsEmpty() ? "script_" + AsString(script_id) : name;
	s.prompt = prompt;
	s.type = "prompt";
	
	scripts.Add(StoreAsJsonValue(s));
	metadata.Set("scripts", scripts);
	SaveMetadata(metadata);
	
	GetAriaLogger("script_manager").Info(Format("Created new script %d", script_id));
	return script_id;
}

Vector<ScriptEntry> ScriptManager::ListScripts() const {
	ValueMap metadata = LoadMetadata();
	ValueArray scripts = metadata["scripts"];
	Vector<ScriptEntry> res;
	for (int i = 0; i < scripts.GetCount(); i++) {
		ScriptEntry s;
		LoadFromJsonValue(s, scripts[i]);
		res.Add(s);
	}
	return res;
}

ScriptEntry ScriptManager::GetScript(const Value& identifier) const {
	Vector<ScriptEntry> scripts = ListScripts();
	if (identifier.Is<int>()) {
		int id = identifier;
		for (const auto& s : scripts) if (s.id == id) return s;
	} else if (IsString(identifier)) {
		String s_id = identifier;
		for (const auto& s : scripts) if (s.name == s_id) return s;
	}
	ScriptEntry s;
	s.id = -1;
	return s;
}

bool ScriptManager::EditScript(const Value& identifier, const String& prompt) {
	ValueMap metadata = LoadMetadata();
	ValueArray scripts = metadata["scripts"];
	bool found = false;
	
	for (int i = 0; i < scripts.GetCount(); i++) {
		ScriptEntry s;
		LoadFromJsonValue(s, scripts[i]);
		bool match = false;
		if (identifier.Is<int>()) match = (s.id == (int)identifier);
		else if (IsString(identifier)) match = (s.name == (String)identifier);
		
		if (match) {
			s.prompt = prompt;
			scripts.Set(i, StoreAsJsonValue(s));
			found = true;
			break;
		}
	}
	
	if (found) {
		metadata.Set("scripts", scripts);
		SaveMetadata(metadata);
		return true;
	}
	return false;
}

bool ScriptManager::RemoveScript(const Value& identifier) {
	ValueMap metadata = LoadMetadata();
	ValueArray scripts = metadata["scripts"];
	int idx = -1;
	
	for (int i = 0; i < scripts.GetCount(); i++) {
		ScriptEntry s;
		LoadFromJsonValue(s, scripts[i]);
		bool match = false;
		if (identifier.Is<int>()) match = (s.id == (int)identifier);
		else if (IsString(identifier)) match = (s.name == (String)identifier);
		
		if (match) {
			idx = i;
			break;
		}
	}
	
	if (idx >= 0) {
		scripts.Remove(idx);
		metadata.Set("scripts", scripts);
		SaveMetadata(metadata);
		return true;
	}
	return false;
}

Vector<String> ScriptManager::GetScriptPlaceholders(const String& prompt) const {
	Vector<String> res;
	const char* p = prompt;
	while (*p) {
		if (p[0] == '{' && p[1] == '{') {
			p += 2;
			const char* s = p;
			while (*p && !(*p == '}' && p[1] == '}')) p++;
			if (*p) {
				String placeholder(s, p);
				if (FindIndex(res, placeholder) < 0) res.Add(placeholder);
				p += 2;
			}
		} else p++;
	}
	return res;
}

String ScriptManager::ApplyParameters(const String& prompt, const ValueMap& parameters) const {
	String res = prompt;
	for (int i = 0; i < parameters.GetCount(); i++) {
		String key = parameters.GetKey(i);
		String val = parameters.GetValue(i);
		res.Replace("{{" + key + "}}", val);
	}
	return res;
}

bool ScriptManager::RunScript(const Value& identifier, AriaNavigator* navigator, ValueMap parameters) {
	ScriptEntry s = GetScript(identifier);
	if (s.id < 0) throw ScriptError("Script not found");
	
	String prompt = s.prompt;
	Vector<String> placeholders = GetScriptPlaceholders(prompt);
	
	for (const String& p : placeholders) {
		if (p.StartsWith("env:")) {
			String env_var = p.Mid(4);
			String val = GetEnv(env_var);
			if (val.IsEmpty() && parameters.Find(p) < 0) {
				// Should prompt but for now just error or use parameter
				if (GetEnv("ARIA_NON_INTERACTIVE") == "true")
					throw ScriptError("Missing environment variable: " + env_var);
			} else if (!val.IsEmpty()) {
				parameters.Set(p, val);
			}
		} else if (p.StartsWith("vault:")) {
			String vault_key = p.Mid(6);
			String val = credential_manager.GetCredential(vault_key);
			if (val.IsEmpty() && parameters.Find(p) < 0) {
				if (GetEnv("ARIA_NON_INTERACTIVE") == "true")
					throw ScriptError("Missing vault key: " + vault_key);
			} else if (!val.IsEmpty()) {
				parameters.Set(p, val);
			}
		}
		
		int q = parameters.Find(p);
		if (q >= 0) {
			AddSecret(parameters.GetValue(q));
		}
	}
	
	prompt = ApplyParameters(prompt, parameters);
	GetAriaLogger("script_manager").Info(Format("Running script %d: %s", s.id, prompt));
	
	if (s.type == "prompt") {
		if (navigator) {
			navigator->NavigateWithPrompt(prompt);
			return true;
		} else throw ScriptError("Navigator not provided");
	}
	return false;
}

END_UPP_NAMESPACE