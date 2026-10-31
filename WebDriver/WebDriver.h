#ifndef _WebDriver_WebDriver_h_
#define _WebDriver_WebDriver_h_

#include <Core/Core.h>
#include "Conversions.h"
#include "Types.h"
#include "By.h"
#include "Capabilities.h"
#include "JsArgs.h"
#include "Element.h"
#include "Errors.h"
#include "Keys.h"
#include "Session.h"
#include "Client.h"
#include "Window.h"
#include "Chrome.h"
#include "Firefox.h"
#include "Ie.h"
#include "Phantom.h"
#include "Wait.h"
#include "WaitMatch.h"
#include "StealthJS.h"
#include "Undetected.h"

NAMESPACE_UPP

// Main WebDriver namespace for U++ implementation
namespace WebDriver {

// The main class for interactions with a server. Automatically connects to a server,
// creates and deletes a session and gives access to session's API.
class Web_driver : public Client, public Session {
public:
	template <class T1, class T2>
	Web_driver(
		const T1& desired,
		const T2& required,
		const String& url,
		detail::Resource::Ownership mode
		)
		: Client(url)
		, Session(CreateSession<T1, T2>(desired, required, mode))
	{}

	template <class T1>
	Web_driver(
		const T1& desired,
		const String& url = K_DEFAULT_WEB_DRIVER_URL,
		detail::Resource::Ownership mode = detail::Resource::IS_OWNER
		)
		: Client(url)
		, Session(CreateSession<T1, Capabilities>(desired, Capabilities(), mode))
	{}
	
	Web_driver(
		const Capabilities& desired = Capabilities(),
		const Capabilities& required = Capabilities(),
		const String& url = K_DEFAULT_WEB_DRIVER_URL,
		detail::Resource::Ownership mode = detail::Resource::IS_OWNER
		)
		: Client(url)
		, Session(CreateSession<Capabilities, Capabilities>(desired, required, mode))
	{}

	Web_driver(
		const String& session_id,
		const String& url = K_DEFAULT_WEB_DRIVER_URL
		)
		: Client(url)
		, Session(AttachSession(session_id))
	{}
};

Web_driver Start(
	const Capabilities& desired,
	const Capabilities& required = Capabilities(),
	const String& url = K_DEFAULT_WEB_DRIVER_URL
	);

Web_driver Start(const Capabilities& desired, const String& url);

} // namespace WebDriver

END_UPP_NAMESPACE

#endif