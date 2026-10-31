#include "WebDriver.h"

NAMESPACE_UPP

Session::Session(const String& id, const detail::Shared<detail::Resource>& resource)
	: session_id_(id)
	, resource_(resource)
	, factory_(new detail::Element_factory())
{
}

Capabilities Session::GetCapabilities() const {
	return resource_->GetValue<Capabilities>("");
}

String Session::GetSource() const {
	return resource_->GetString("source");
}

String Session::GetTitle() const {
	return resource_->GetString("title");
}

String Session::GetUrl() const {
	return resource_->GetString("url");
}

String Session::GetScreenshot() const {
	return resource_->GetString("screenshot");
}

const Session& Session::Navigate(const String& url) const {
	resource_->Post("url", "url", url);
	return *this;
}

const Session& Session::Get(const String& url) const {
	return Navigate(url);
}

const Session& Session::Forward() const {
	resource_->Post("forward");
	return *this;
}

const Session& Session::Back() const {
	resource_->Post("back");
	return *this;
}

const Session& Session::Refresh() const {
	resource_->Post("refresh");
	return *this;
}

Element Session::GetActiveElement() const {
	detail::Finder finder(resource_, factory_);
	return finder.FindElement(By::CssSelector(":focus")); // Simplified, actual active element might need different command
}

Element Session::FindElement(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.FindElement(by);
}

Vector<Element> Session::FindElements(const By& by) const {
	detail::Finder finder(resource_, factory_);
	return finder.FindElements(by);
}

void Session::DeleteSession() const {
	resource_->Delete();
}

const Session& Session::Execute(const String& script, const JsArgs& args) const {
	Value result;
	try {
		InternalEval("execute/sync", script, args, result);
	} catch (const std::exception& e) {
		// Fallback for legacy drivers
		InternalEval("execute", script, args, result);
	}
	return *this;
}

const Session& Session::ExecuteAsync(const String& script, const JsArgs& args) const {
	Value result;
	try {
		InternalEval("execute/async", script, args, result);
	} catch (const std::exception& e) {
		// Fallback for legacy drivers
		InternalEval("execute_async", script, args, result);
	}
	return *this;
}

const Session& Session::SetFocusToFrame(const Element& frame) const {
	return InternalSetFocusToFrame(ToJson(frame));
}

const Session& Session::SetFocusToFrame(const String& id) const {
	return InternalSetFocusToFrame(id);
}

const Session& Session::SetFocusToFrame(int number) const {
	return InternalSetFocusToFrame(number);
}

const Session& Session::SetFocusToDefaultFrame() const {
	return InternalSetFocusToFrame(Value());
}

const Session& Session::SetFocusToParentFrame() const {
	resource_->Post("frame/parent");
	return *this;
}

Vector<Window> Session::GetWindows() const {
	ValueArray handles = resource_->GetValue<ValueArray>("window/handles");
	Vector<Window> windows;
	for (const auto& handle : handles) {
		windows.Add(MakeWindow(handle));
	}
	return windows;
}

Window Session::GetCurrentWindow() const {
	return MakeWindow(resource_->GetString("window"));
}

const Session& Session::CloseCurrentWindow() const {
	resource_->Delete("window");
	return *this;
}

const Session& Session::SetFocusToWindow(const String& window_name) const {
	resource_->Post("window", "handle", window_name);
	return *this;
}

const Session& Session::SetFocusToWindow(const Window& window) const {
	return SetFocusToWindow(window.GetHandle());
}

Vector<Cookie> Session::GetCookies() const {
	return resource_->GetValue<Vector<Cookie>>("cookie");
}

const Session& Session::SetCookie(const Cookie& cookie) const {
	resource_->Post("cookie", "cookie", ToJson(cookie));
	return *this;
}

const Session& Session::DeleteCookies() const {
	resource_->Delete("cookie");
	return *this;
}

const Session& Session::DeleteCookie(const String& name) const {
	resource_->Delete("cookie/" + name);
	return *this;
}

String Session::GetAlertText() const {
	return resource_->GetString("alert/text");
}

const Session& Session::SendKeysToAlert(const String& text) const {
	resource_->Post("alert/text", "text", text);
	return *this;
}

const Session& Session::AcceptAlert() const {
	resource_->Post("alert/accept");
	return *this;
}

const Session& Session::DismissAlert() const {
	resource_->Post("alert/dismiss");
	return *this;
}

const Session& Session::SendKeys(const String& keys) const {
	resource_->Post("keys", "value", ValueArray() << keys);
	return *this;
}

const Session& Session::SendKeys(const Shortcut& shortcut) const {
	ValueArray arr;
	for (const auto& key : shortcut.keys) arr << key;
	resource_->Post("keys", "value", arr);
	return *this;
}

const Session& Session::MoveToTopLeftOf(const Element& element, const Offset& offset) const {
	return InternalMoveTo(&element, &offset);
}

const Session& Session::MoveToCenterOf(const Element& element) const {
	return InternalMoveTo(&element, nullptr);
}

const Session& Session::MoveTo(const Offset& offset) const {
	return InternalMoveTo(nullptr, &offset);
}

const Session& Session::Click(mouse::Button button) const {
	return InternalMouseButtonCommand("click", button);
}

const Session& Session::DoubleClick() const {
	resource_->Post("doubleclick");
	return *this;
}

const Session& Session::ButtonDown(mouse::Button button) const {
	return InternalMouseButtonCommand("buttondown", button);
}

const Session& Session::ButtonUp(mouse::Button button) const {
	return InternalMouseButtonCommand("buttonup", button);
}

const Session& Session::SetTimeoutMs(timeout::Type type, int milliseconds) {
	ValueMap m;
	m.Add("type", type);
	m.Add("ms", milliseconds);
	resource_->Post("timeouts", m);
	return *this;
}

const Session& Session::SetImplicitTimeoutMs(int milliseconds) {
	return SetTimeoutMs(timeout::Implicit, milliseconds);
}

const Session& Session::SetAsyncScriptTimeoutMs(int milliseconds) {
	return SetTimeoutMs(timeout::Script, milliseconds);
}

const Session& Session::ApplyStealthJS() const {
	Execute(GetStealthJS(), JsArgs());
	return *this;
}

Window Session::MakeWindow(const String& handle) const {
	return Window(handle, detail::MakeSubResource(resource_, "window"));
}

detail::Keyboard Session::GetKeyboard() const {
	return detail::Keyboard();
}

void Session::InternalEval(const String& webdriver_command,
	const String& script, const JsArgs& args,
	Element& result) const {
	Value v = InternalEvalJsonValue(webdriver_command, script, args);
	String id = FromJson<String>(v["ELEMENT"]);
	result = Element(id, detail::MakeSubResource(resource_, "element/" + id), factory_);
}

Value Session::InternalEvalJsonValue(const String& command,
	const String& script, const JsArgs& args) const {
	ValueMap m;
	m.Add("script", script);
	m.Add("args", ToJson(args));
	return resource_->Post(command, m);
}

const Session& Session::InternalSetFocusToFrame(const Value& id) const {
	resource_->Post("frame", "id", id);
	return *this;
}

const Session& Session::InternalMoveTo(const Element* element, const Offset* offset) const {
	ValueMap m;
	if (element) m.Add("element", element->GetRef());
	if (offset) {
		m.Add("xoffset", offset->x);
		m.Add("yoffset", offset->y);
	}
	resource_->Post("moveto", m);
	return *this;
}

const Session& Session::InternalMouseButtonCommand(const char* command, mouse::Button button) const {
	resource_->Post(command, "button", (int)button);
	return *this;
}

END_UPP_NAMESPACE