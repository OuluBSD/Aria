#ifndef _WebDriver_Session_h_
#define _WebDriver_Session_h_

#include <Core/Core.h>
#include "detail.h"

NAMESPACE_UPP

class Client;

class Session : public Moveable<Session> { // copyable
public:
	Capabilities GetCapabilities() const;
	String GetSource() const;
	String GetTitle() const;
	String GetUrl() const;
	String GetScreenshot() const; // Base64 PNG

	const Session& Navigate(const String& url) const;
	const Session& Get(const String& url) const; // Same as Navigate
	const Session& Forward() const;
	const Session& Back() const;
	const Session& Refresh() const;

	const Session& Execute(const String& script, const JsArgs& args = JsArgs()) const;
	template<typename T>
	T Eval(const String& script, const JsArgs& args = JsArgs()) const {
		T result;
		try {
			InternalEval("execute/sync", script, args, result);
		} catch (const std::exception& e) {
			InternalEval("execute", script, args, result);
		}
		return result;
	}
	const Session& ExecuteAsync(const String& script, const JsArgs& args = JsArgs()) const;
	template<typename T>
	T EvalAsync(const String& script, const JsArgs& args = JsArgs()) const {
		T result;
		try {
			InternalEval("execute/async", script, args, result);
		} catch (const std::exception& e) {
			InternalEval("execute_async", script, args, result);
		}
		return result;
	}

	const Session& SetFocusToFrame(const Element& frame) const;
	const Session& SetFocusToFrame(const String& id) const;
	const Session& SetFocusToFrame(int number) const;
	const Session& SetFocusToDefaultFrame() const;
	const Session& SetFocusToParentFrame() const;

	Vector<Window> GetWindows() const;
	Window GetCurrentWindow() const;
	const Session& CloseCurrentWindow() const;
	const Session& SetFocusToWindow(const String& window_name) const;
	const Session& SetFocusToWindow(const Window& window) const;

	Element GetActiveElement() const;

	Element FindElement(const By& by) const;
	Vector<Element> FindElements(const By& by) const;

	Vector<Cookie> GetCookies() const;
	const Session& SetCookie(const Cookie& cookie) const;
	const Session& DeleteCookies() const;
	const Session& DeleteCookie(const String& name) const;

	String GetAlertText() const;
	const Session& SendKeysToAlert(const String& text) const;
	const Session& AcceptAlert() const;
	const Session& DismissAlert() const;

	const Session& SendKeys(const String& keys) const;
	const Session& SendKeys(const Shortcut& shortcut) const;

	const Session& MoveToTopLeftOf(const Element&, const Offset& = Offset()) const;
	const Session& MoveToCenterOf(const Element&) const;
	const Session& MoveTo(const Offset&) const;
	const Session& Click(mouse::Button = mouse::LEFT_BUTTON) const;
	const Session& DoubleClick() const;
	const Session& ButtonDown(mouse::Button = mouse::LEFT_BUTTON) const;
	const Session& ButtonUp(mouse::Button = mouse::LEFT_BUTTON) const;

	const Session& SetTimeoutMs(timeout::Type type, int milliseconds);
	const Session& SetImplicitTimeoutMs(int milliseconds);
	const Session& SetAsyncScriptTimeoutMs(int milliseconds);

	const Session& ApplyStealthJS() const;

	void DeleteSession() const; // No need to delete sessions created by WebDriver or Client
	virtual ~Session() {}

	String GetSessionId() const { return session_id_; }

private:
	friend class Client; // Only Client can create Sessions

	explicit Session(const String& id, const detail::Shared<detail::Resource>& resource);

	Window MakeWindow(const String& handle) const;
	detail::Keyboard GetKeyboard() const;
	template<typename T>
	void InternalEval(const String& webdriver_command,
		const String& script, const JsArgs& args,
		T& result) const {
		result = FromJson<T>(InternalEvalJsonValue(webdriver_command, script, args));
	}
	void InternalEval(const String& webdriver_command,
		const String& script, const JsArgs& args,
		Element& result) const;
	Value InternalEvalJsonValue(const String& command,
		const String& script, const JsArgs& args) const;
	const Session& InternalSetFocusToFrame(const Value& id) const;
	const Session& InternalMoveTo(const Element*, const Offset*) const;
	const Session& InternalMouseButtonCommand(const char* command, mouse::Button button) const;

private:
	String session_id_;
	detail::Shared<detail::Resource> resource_;
	detail::Shared<detail::IFinder_factory> factory_;
};

END_UPP_NAMESPACE

#endif