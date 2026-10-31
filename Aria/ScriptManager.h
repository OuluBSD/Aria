#ifndef _Aria_ScriptManager_h_
#define _Aria_ScriptManager_h_

struct ScriptEntry : public Moveable<ScriptEntry> {
	int id;
	String name;
	String prompt;
	String type;
	
	void Jsonize(JsonIO& json) {
		json("id", id)("name", name)("prompt", prompt)("type", type);
	}
};

class ScriptManager {
	String scripts_dir;
	String metadata_file;
	CredentialManager credential_manager;
	
	ValueMap LoadMetadata() const;
	void SaveMetadata(const ValueMap& metadata);

public:
	ScriptManager();
	
	int CreateScript(const String& prompt, const String& name = "");
	Vector<ScriptEntry> ListScripts() const;
	ScriptEntry GetScript(const Value& identifier) const;
	bool EditScript(const Value& identifier, const String& prompt);
	bool RemoveScript(const Value& identifier);
	
	bool RunScript(const Value& identifier, AriaNavigator* navigator = nullptr, ValueMap parameters = ValueMap());
	
	Vector<String> GetScriptPlaceholders(const String& prompt) const;
	String ApplyParameters(const String& prompt, const ValueMap& parameters) const;
};

#endif
