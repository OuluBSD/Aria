#ifndef _WebDriver_Element_h_
#define _WebDriver_Element_h_

#include <Core/Core.h>
#include "detail.h"



NAMESPACE_UPP

// An element from DOM
class Element : public Moveable<Element> { // copyable
public:
	Element();

	Element(
		const String& ref,
		const detail::Shared<detail::Resource>& resource,
		const detail::Shared<detail::IFinder_factory>& factory
		);

	String GetRef() const; // Returns ID that is used by Webdriver to identify elements

	bool IsDisplayed() const;
	bool IsEnabled() const;
	bool IsSelected() const;
	Point GetLocation() const;
	Point GetLocationInView() const;
	Size GetSize() const;
	String GetAttribute(const String& name) const;
	String GetCssProperty(const String& name) const;
	String GetTagName() const;
	String GetText() const;

	Element FindElement(const By& by) const;
	Vector<Element> FindElements(const By& by) const;

	const Element& Clear() const;
	const Element& Click() const;
	const Element& Submit() const;

	const Element& SendKeys(const String& keys) const;
	const Element& SendKeys(const Shortcut& shortcut) const;

	bool Equals(const Element& other) const;
	bool operator != (const Element& other) const;
	bool operator == (const Element& other) const;
	bool operator < (const Element& other) const;

	void Jsonize(JsonIO& json) {
		json("ELEMENT", ref_)("element-6066-11e4-a52e-4f735466cecf", ref_);
	}

private:
	detail::Resource& GetResource() const;
	detail::Keyboard GetKeyboard() const;

private:
	String ref_;
	detail::Shared<detail::Resource> resource_;
	detail::Shared<detail::IFinder_factory> factory_;
};

END_UPP_NAMESPACE

#endif