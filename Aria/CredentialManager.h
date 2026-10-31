#ifndef _Aria_CredentialManager_h_
#define _Aria_CredentialManager_h_

class CredentialManager {
	String aria_dir;
	String credentials_file;
	
	ValueMap LoadCredentials() const;
	void SaveCredentials(const ValueMap& credentials);

public:
	CredentialManager();
	
	void SetCredential(const String& key, const String& value);
	String GetCredential(const String& key) const;
	Vector<String> ListKeys() const;
	bool RemoveCredential(const String& key);
};

#endif
