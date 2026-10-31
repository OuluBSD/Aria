#ifndef _WebDriver_Keyboard_h_
#define _WebDriver_Keyboard_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace detail {

class Keyboard {
public:
	Keyboard();
	
	void SendKeys(const String& keys) const;
	void SendModifierKeys(const String& modifiers) const;
	
private:
	String current_keys_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif