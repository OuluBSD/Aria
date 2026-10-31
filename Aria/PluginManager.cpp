#include "Aria.h"

NAMESPACE_UPP

PluginManager::PluginManager(const String& plugins_dir) {
	if (plugins_dir.IsEmpty()) {
		this->plugins_dir = GetHomeDirFile(AppendFileName(".aria", "plugins"));
	} else {
		this->plugins_dir = plugins_dir;
	}
	RealizeDirectory(this->plugins_dir);
}

void PluginManager::LoadPlugins() {
	GetAriaLogger("plugin_manager").Info("Loading plugins from " + plugins_dir);
	// Placeholder for dynamic loading
}

void PluginManager::RegisterPlugin(BasePlugin* plugin) {
	One<BasePlugin> p(plugin);
	p->OnLoad();
	
	VectorMap<String, BaseAIProvider*> providers = p->GetAIProviders();
	for (int i = 0; i < providers.GetCount(); i++) {
		ai_providers.GetAdd(providers.GetKey(i)) = providers[i];
	}
	
	plugins.Add(pick(p));
}

BaseAIProvider* PluginManager::GetAIProvider(const String& name) {
	return ai_providers.Get(name, nullptr);
}

Vector<String> PluginManager::ListAIProviders() const {
	Vector<String> res;
	for (int i = 0; i < ai_providers.GetCount(); i++) {
		res.Add(ai_providers.GetKey(i));
	}
	return res;
}

void PluginManager::TriggerHook(const String& hook_name, const ValueMap& args) {
	// Placeholder for hook execution
}

END_UPP_NAMESPACE