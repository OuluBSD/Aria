#include "WebDriver.h"

NAMESPACE_UPP

namespace WebDriver {

Web_driver Start(
	const Capabilities& desired,
	const Capabilities& required,
	const String& url
	)
{
	return Web_driver(desired, required, url);
}

Web_driver Start(const Capabilities& desired, const String& url)
{
	return Start(desired, Capabilities(), url);
}

} // namespace WebDriver

END_UPP_NAMESPACE