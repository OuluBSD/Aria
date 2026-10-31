#include "Aria.h"

NAMESPACE_UPP

CredentialManager::CredentialManager() {
	aria_dir = GetHomeDirFile(".aria");
	credentials_file = AppendFileName(aria_dir, "credentials.json");
	
	RealizeDirectory(aria_dir);
	
	if (!FileExists(credentials_file)) {
		SaveCredentials(ValueMap());
#ifdef PLATFORM_POSIX
		chmod(credentials_file, 0600);
#endif
	}
}

ValueMap CredentialManager::LoadCredentials() const {
	if (!FileExists(credentials_file)) return ValueMap();
	String json = LoadFile(credentials_file);
	Value v = ParseJSON(json);
	if (v.Is<ValueMap>()) return v;
	return ValueMap();
}

void CredentialManager::SaveCredentials(const ValueMap& credentials) {
	String json = AsJSON(credentials, true);
	if (!SaveFile(credentials_file, json)) {
		GetAriaLogger("credential_manager").Error("Failed to save credentials to " + credentials_file);
	}
}

void CredentialManager::SetCredential(const String& key, const String& value) {
	ValueMap credentials = LoadCredentials();
	credentials.Set(key, value);
	SaveCredentials(credentials);
	GetAriaLogger("credential_manager").Info("Set credential: " + key);
}

String CredentialManager::GetCredential(const String& key) const {
	ValueMap credentials = LoadCredentials();
	return (String)credentials[key];
}

Vector<String> CredentialManager::ListKeys() const {
	ValueMap credentials = LoadCredentials();
	Vector<String> keys;
	for (int i = 0; i < credentials.GetCount(); i++) {
		keys.Add(credentials.GetKey(i));
	}
	return keys;
}

bool CredentialManager::RemoveCredential(const String& key) {
	ValueMap credentials = LoadCredentials();
	if (credentials.Find(key) >= 0) {
		credentials.RemoveKey(key);
		SaveCredentials(credentials);
		GetAriaLogger("credential_manager").Info("Removed credential: " + key);
		return true;
	}
	return false;
}

END_UPP_NAMESPACE
