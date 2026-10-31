#include "WebDriver.h"

NAMESPACE_UPP

Client::Client(const String& url) {
	// Initialize client with the given URL
	resource_ = detail::Shared<detail::Resource>(new detail::Resource(url));
}

Value Client::GetStatus() const {
	// Return server status
	Value status = resource_->Get("status");
	// Already in Upp::Value format
	return status;
}

Vector<Session> Client::GetSessions() const {
	// Return existing sessions
	Value result = resource_->Get("sessions");
	ValueArray session_values = FromJson<ValueArray>(result);
	Vector<Session> sessions;
	for(const auto& val : session_values) {
		Value id_val = val["id"];
		String session_id = FromJson<String>(id_val);
		sessions.Add(Session(session_id, detail::Shared<detail::Resource>(
			new detail::Resource(resource_, session_id, detail::Resource::IS_OBSERVER))));
	}
	return sessions;
}

Session Client::MakeSession(
	const String& id,
	detail::Resource::Ownership mode
	) const {
	return Session(id, detail::Shared<detail::Resource>(
		new detail::Resource(resource_, "session/" + id, mode)));
}

END_UPP_NAMESPACE