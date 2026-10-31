#ifndef _WebDriver_By_h_
#define _WebDriver_By_h_

#include <Core/Core.h>

NAMESPACE_UPP

class By { // copyable
public:
	By(const String& method, const String& selector);

	static By Id(const String& id);
	static By Name(const String& name);
	static By ClassName(const String& class_name);
	static By TagName(const String& tag_name);
	static By CssSelector(const String& css_selector);
	static By Xpath(const String& xpath);
	static By LinkText(const String& link_text);
	static By PartialLinkText(const String& partial_link_text);
	static By Text(const String& text);

	const String& GetMethod() const;
	const String& GetSelector() const;

	void Jsonize(JsonIO& json) {
		json("using", method_)("value", selector_);
	}

private:
	String method_;
	String selector_;
};

END_UPP_NAMESPACE

#endif