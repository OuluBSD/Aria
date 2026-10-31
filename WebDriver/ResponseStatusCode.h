#ifndef _WebDriver_ResponseStatusCode_h_
#define _WebDriver_ResponseStatusCode_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace response_status_code {

enum Value {
	kSuccess = 0,
	kNoSuchElement = 7,
	kNoSuchFrame = 8,
	kUnknownCommand = 9,
	kStaleElementReference = 10,
	kElementNotVisible = 11,
	kInvalidElementState = 12,
	kUnknownError = 13,
	kElementIsNotSelectable = 15,
	kJavaScriptError = 17,
	kXPathLookupError = 19,
	kTimeout = 21,
	kNoSuchWindow = 23,
	kInvalidCookieDomain = 24,
	kUnableToSetCookie = 25,
	kUnexpectedAlertOpen = 26,
	kNoAlertOpenError = 27,
	kScriptTimeout = 28,
	kInvalidElementCoordinates = 29,
	kIMENotAvailable = 30,
	kIMEEngineActivationFailed = 31,
	kInvalidSelector = 32,
	kSessionNotCreatedException = 33,
	kMoveTargetOutOfBounds = 34
};

const char* ToString(Value code);

} // namespace response_status_code

END_UPP_NAMESPACE

#endif