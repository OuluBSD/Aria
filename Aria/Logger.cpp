#include "Aria.h"

NAMESPACE_UPP

static Vector<String>& Secrets() {
	static Vector<String> s;
	return s;
}

void AddSecret(const String& secret) {
	if (secret.IsEmpty() || secret.GetLength() <= 3) return;
	Vector<String>& s = Secrets();
	if (FindIndex(s, secret) < 0) {
		s.Add(secret);
		// Sort by length descending to avoid partial redaction
		Sort(s, [](const String& a, const String& b) { return a.GetLength() > b.GetLength(); });
	}
}

String Redact(const String& text) {
	String res = text;
	for (const String& secret : Secrets()) {
		res.Replace(secret, "[REDACTED]");
	}
	return res;
}

void AriaLogger::Info(const String& msg) {
	RLOG(name << ": " << Redact(msg));
}

void AriaLogger::Warning(const String& msg) {
	RLOG(name << ": WARNING: " << Redact(msg));
}

void AriaLogger::Error(const String& msg) {
	RLOG(name << ": ERROR: " << Redact(msg));
}

void AriaLogger::Debug(const String& msg) {
#ifdef _DEBUG
	RLOG(name << ": DEBUG: " << Redact(msg));
#endif
}

AriaLogger& GetAriaLogger(const String& name) {
	static VectorMap<String, One<AriaLogger>> loggers;
	int q = loggers.Find(name);
	if (q < 0) {
		q = loggers.FindAdd(name, new AriaLogger(name));
	}
	return *loggers[q];
}

END_UPP_NAMESPACE
