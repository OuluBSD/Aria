#ifndef _Aria_Exceptions_h_
#define _Aria_Exceptions_h_

struct AriaError : public Exc {
	AriaError(const String& s) : Exc(s) {}
};

struct BrowserError : public AriaError {
	BrowserError(const String& s) : AriaError(s) {}
};

struct SessionError : public BrowserError {
	SessionError(const String& s) : BrowserError(s) {}
};

struct NavigationError : public BrowserError {
	NavigationError(const String& s) : BrowserError(s) {}
};

struct ScriptError : public AriaError {
	ScriptError(const String& s) : AriaError(s) {}
};

struct AIServiceError : public AriaError {
	AIServiceError(const String& s) : AriaError(s) {}
};

struct ReportError : public AriaError {
	ReportError(const String& s) : AriaError(s) {}
};

#endif