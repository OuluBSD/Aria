#ifndef _WebDriver_JsArgs_h_
#define _WebDriver_JsArgs_h_

#include <Core/Core.h>


NAMESPACE_UPP

class JsArgs : public Moveable<JsArgs> { // copyable
public:
	JsArgs();

	template<typename T>
	JsArgs(const T& value);

	template<typename T>
	JsArgs& Add(const T& value);

	Value operator[](int index) const;

	int Size() const;

	void Jsonize(JsonIO& json) {
		json.Set(ToJson(args_));
	}

private:
	ValueArray args_;
};

template<typename T>
JsArgs::JsArgs(const T& value) {
	Add(value);
}

template<typename T>
JsArgs& JsArgs::Add(const T& value) {
	args_.Add(ToJson(value));
	return *this;
}

END_UPP_NAMESPACE

#endif