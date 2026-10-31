#ifndef _WebDriver_Client_h_
#define _WebDriver_Client_h_

#include <Core/Core.h>

NAMESPACE_UPP

const char *const K_DEFAULT_WEB_DRIVER_URL = "http://localhost:4444/";

// Gives low level access to server's resources. You normally should not use it.
class Client { // copyable
public:
	explicit Client(const String& url = String(K_DEFAULT_WEB_DRIVER_URL));
	virtual ~Client() {}

	Value GetStatus() const;

	// Returns existing sessions.
	Vector<Session> GetSessions() const;

	// Creates new session.
	template <class T1, class T2>
	Session CreateSession(
		const T1& desired,
		const T2& required,
		detail::Resource::Ownership mode = detail::Resource::IS_OWNER
		) const {
		ValueMap caps;
		// Modern W3C format
		ValueMap always_match = ToJson(desired);
		ValueMap capabilities;
		capabilities.Add("alwaysMatch", always_match);
		caps.Add("capabilities", capabilities);
		
		Value response = resource_->Post("session", caps);
		
		String session_id;
		if (response.Is<ValueMap>()) {
			if (!response["sessionId"].IsVoid())
				session_id = FromJson<String>(response["sessionId"]);
			else if (!response["value"].IsVoid() && response["value"].Is<ValueMap>()) {
				Value v = response["value"];
				if (!v["sessionId"].IsVoid())
					session_id = FromJson<String>(v["sessionId"]);
			}
		}
		
		if (session_id.IsEmpty())
			throw Exc("Failed to extract sessionId from server response. Response: " + AsJSON(response));

		return MakeSession(session_id, mode);
	}

	Session CreateSession(
		const Capabilities& desired,
		const Capabilities& required,
		detail::Resource::Ownership mode
		) const {
		return CreateSession<Capabilities, Capabilities>(desired, required, mode);
	}

	Session CreateSession(
		const Capabilities& desired = Capabilities(),
		const Capabilities& required = Capabilities()
		) const {
		return CreateSession<Capabilities, Capabilities>(desired, required, detail::Resource::IS_OWNER);
	}

	// Connects to an existing session.
	Session AttachSession(const String& session_id) const {
		return MakeSession(session_id, detail::Resource::IS_OBSERVER);
	}

private:
	Session MakeSession(
		const String& id,
		detail::Resource::Ownership mode
		) const;

private:
	detail::Shared<detail::Resource> resource_;
};

END_UPP_NAMESPACE

#endif