#ifndef _WebDriver_Window_h_
#define _WebDriver_Window_h_

#include <Core/Core.h>


NAMESPACE_UPP

class Window : public Moveable<Window> { // copyable
public:
	Window();

	Window(
		const String& handle,
		const detail::Shared<detail::Resource>& resource
		);

	String GetHandle() const;

	Size GetSize() const;
	const Window& SetSize(const Size& size) const;

	Point GetPosition() const;
	const Window& SetPosition(const Point& position) const;

	const Window& Maximize() const;
	const Window& Fullscreen() const;

	const Window& Close() const;
	String GetTitle() const;
	String GetUrl() const;

private:
	String handle_;
	detail::Shared<detail::Resource> resource_;
};

END_UPP_NAMESPACE

#endif
