#include "Aria.h"

NAMESPACE_UPP

UndetectedFirefox::UndetectedFirefox(const Capabilities& caps)
	: WebDriver::Web_driver(caps)
{
	// Additional options to make the browser less detectable
	// In Python: options.set_preference("media.volume_scale", "0.0")
	// In C++, we'd need to send these via capabilities or specific commands
}

END_UPP_NAMESPACE
