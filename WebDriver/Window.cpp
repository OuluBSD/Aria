#include "WebDriver.h"

NAMESPACE_UPP

Window::Window() {
}

Window::Window(
	const String& handle,
	const detail::Shared<detail::Resource>& resource
	)
	: handle_(handle)
	, resource_(resource)
{
}

String Window::GetHandle() const {
	return handle_;
}

Size Window::GetSize() const {
	return FromJson<Size>(resource_->Get("size"));
}

const Window& Window::SetSize(const Size& size) const {
	resource_->Post("size", ToJson(size));
	return *this;
}

Point Window::GetPosition() const {
	return FromJson<Point>(resource_->Get("position"));
}

const Window& Window::SetPosition(const Point& position) const {
	resource_->Post("position", ToJson(position));
	return *this;
}

const Window& Window::Maximize() const {
	resource_->Post("maximize");
	return *this;
}

const Window& Window::Fullscreen() const {
	resource_->Post("fullscreen");
	return *this;
}

const Window& Window::Close() const {
	resource_->Delete();
	return *this;
}

String Window::GetTitle() const {
	return resource_->GetString("title");
}

String Window::GetUrl() const {
	return resource_->GetString("url");
}

END_UPP_NAMESPACE