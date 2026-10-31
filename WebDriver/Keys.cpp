#include "WebDriver.h"

NAMESPACE_UPP

Shortcut::Shortcut(const String& key) {
	keys.Add(key);
}

Shortcut::Shortcut(const Vector<String>& keys) {
	this->keys <<= keys;
}

END_UPP_NAMESPACE