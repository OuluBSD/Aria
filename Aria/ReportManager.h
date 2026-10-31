#ifndef _Aria_ReportManager_h_
#define _Aria_ReportManager_h_

class ReportManager {
	String reports_dir;
public:
	ReportManager(const String& reports_dir = "");
	
	String GenerateMarkdownReport(const String& title, const String& content, const Vector<String>& sources = Vector<String>(), const ValueArray& metrics = ValueArray());
	String GenerateHtmlReport(const String& title, const String& content, const Vector<String>& sources = Vector<String>(), const ValueArray& metrics = ValueArray());
	
	const String& GetReportsDir() const { return reports_dir; }
};

#endif
