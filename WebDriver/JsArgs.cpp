#include "WebDriver.h"

NAMESPACE_UPP

JsArgs::JsArgs() {
}

Value JsArgs::operator[](int index) const {
	if (index >= 0 && index < args_.GetCount()) {
		return args_[index];
	}
	return Value();
}

int JsArgs::Size() const {
	return static_cast<int>(args_.GetCount());
}

END_UPP_NAMESPACE