#include "WebDriver.h"

NAMESPACE_UPP

namespace response_status_code {

const char* ToString(Value code) {
	switch (code) {
		case kSuccess: return "Success";
		case kNoSuchElement: return "NoSuchElement";
		case kNoSuchFrame: return "NoSuchFrame";
		case kUnknownCommand: return "UnknownCommand";
		case kStaleElementReference: return "StaleElementReference";
		case kElementNotVisible: return "ElementNotVisible";
		case kInvalidElementState: return "InvalidElementState";
		case kUnknownError: return "UnknownError";
		case kElementIsNotSelectable: return "ElementIsNotSelectable";
		case kJavaScriptError: return "JavaScriptError";
		case kXPathLookupError: return "XPathLookupError";
		case kTimeout: return "Timeout";
		case kNoSuchWindow: return "NoSuchWindow";
		case kInvalidCookieDomain: return "InvalidCookieDomain";
		case kUnableToSetCookie: return "UnableToSetCookie";
		case kUnexpectedAlertOpen: return "UnexpectedAlertOpen";
		case kNoAlertOpenError: return "NoAlertOpenError";
		case kScriptTimeout: return "ScriptTimeout";
		case kInvalidElementCoordinates: return "InvalidElementCoordinates";
		case kIMENotAvailable: return "IMENotAvailable";
		case kIMEEngineActivationFailed: return "IMEEngineActivationFailed";
		case kInvalidSelector: return "InvalidSelector";
		case kSessionNotCreatedException: return "SessionNotCreatedException";
		case kMoveTargetOutOfBounds: return "MoveTargetOutOfBounds";
		default: return "UnknownErrorCode";
	}
}

} // namespace response_status_code

END_UPP_NAMESPACE