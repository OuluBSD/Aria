#ifndef _Aria_Logger_h_
#define _Aria_Logger_h_

void AddSecret(const String& secret);
String Redact(const String& text);

class AriaLogger {
	String name;
public:
	AriaLogger(const String& name) : name(name) {}
	
	void Info(const String& msg);
	void Warning(const String& msg);
	void Error(const String& msg);
	void Debug(const String& msg);
};

AriaLogger& GetAriaLogger(const String& name);

#endif