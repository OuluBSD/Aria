#ifndef _Aria_PluginManager_h_
#define _Aria_PluginManager_h_

#include "BaseAIProvider.h"

class BasePlugin {
public:
	virtual ~BasePlugin() {}
	virtual void OnLoad() {}
	virtual VectorMap<String, BaseAIProvider*> GetAIProviders() { return VectorMap<String, BaseAIProvider*>(); }
};

class PluginManager {
	Vector<One<BasePlugin>> plugins;
	VectorMap<String, BaseAIProvider*> ai_providers;
	String plugins_dir;
public:
	PluginManager(const String& plugins_dir = "");
	
	void LoadPlugins();
	void RegisterPlugin(BasePlugin* plugin);
	
	BaseAIProvider* GetAIProvider(const String& name);
	Vector<String> ListAIProviders() const;
	
	void TriggerHook(const String& hook_name, const ValueMap& args);
};

#endif
