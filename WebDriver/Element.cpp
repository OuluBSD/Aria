#include "WebDriver.h"

NAMESPACE_UPP

Element::Element()
	: ref_()
{
}

Element::Element(
	const String& ref,
	const detail::Shared<detail::Resource>& resource,
	const detail::Shared<detail::IFinder_factory>& factory
	)
	: ref_(ref)
	, resource_(resource)
	, factory_(factory)
{
}

String Element::GetRef() const {
	return ref_;
}

bool Element::IsDisplayed() const {
	return resource_->GetBool("displayed");
}

bool Element::IsEnabled() const {
	return resource_->GetBool("enabled");
}

bool Element::IsSelected() const {
	return resource_->GetBool("selected");
}

Point Element::GetLocation() const {
	return FromJson<Point>(resource_->Get("location"));
}

Point Element::GetLocationInView() const {
	return FromJson<Point>(resource_->Get("location_in_view"));
}

Size Element::GetSize() const {
	return FromJson<Size>(resource_->Get("size"));
}

String Element::GetAttribute(const String& name) const {
	return resource_->GetValue<String>("attribute/" + name);
}

String Element::GetCssProperty(const String& name) const {
	return resource_->GetValue<String>("css/" + name);
}

String Element::GetTagName() const {
	return resource_->GetString("name");
}

String Element::GetText() const {
	return resource_->GetString("text");
}

Element Element::FindElement(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.FindElement(by);
}

Vector<Element> Element::FindElements(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.FindElements(by);
}

const Element& Element::Clear() const {
	resource_->Post("clear");
	return *this;
}

const Element& Element::Click() const {
	resource_->Post("click");
	return *this;
}

const Element& Element::Submit() const {
	resource_->Post("submit");
	return *this;
}

const Element& Element::SendKeys(const String& keys) const {
	resource_->Post("value", "value", keys);
	return *this;
}

const Element& Element::SendKeys(const Shortcut& shortcut) const {
	// Convert shortcut to string representation and send
	String keys;
	for (const auto& key : shortcut.keys) {
		keys += key;
	}
	SendKeys(keys);
	return *this;
}

bool Element::Equals(const Element& other) const {
	return ref_ == other.ref_;
}

bool Element::operator!=(const Element& other) const {
	return !(*this == other);
}

bool Element::operator==(const Element& other) const {
	return Equals(other);
}

bool Element::operator<(const Element& other) const {
	return ref_ < other.ref_;
}

detail::Resource& Element::GetResource() const {
	return *resource_;
}

detail::Keyboard Element::GetKeyboard() const {
	return detail::Keyboard();
}

END_UPP_NAMESPACE