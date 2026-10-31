#ifndef _Aria_UndetectedFirefox_h_
#define _Aria_UndetectedFirefox_h_

class UndetectedFirefox : public WebDriver::Web_driver {
public:
	UndetectedFirefox(const Capabilities& caps = Capabilities());
};

#endif
