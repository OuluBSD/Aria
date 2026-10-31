#include "Aria.h"

NAMESPACE_UPP

void CreateUndetectedFirefoxSession(bool headless, const String& profile_path) {
	AriaNavigator navigator;
	navigator.StartSession("firefox", headless);
}

END_UPP_NAMESPACE
