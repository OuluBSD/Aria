#include "Aria.h"

NAMESPACE_UPP

ReportManager::ReportManager(const String& reports_dir) {
	if (reports_dir.IsEmpty()) {
		this->reports_dir = GetHomeDirFile(AppendFileName(".aria", "reports"));
	} else {
		this->reports_dir = reports_dir;
	}
	RealizeDirectory(this->reports_dir);
}

static String GetSafeTitle(const String& title) {
	String res;
	for (int i = 0; i < title.GetLength(); i++) {
		int c = title[i];
		if (IsAlNum(c) || c == ' ' || c == '_') res.Cat(c);
	}
	String safe = TrimBoth(res);
	safe.Replace(" ", "_");
	return safe;
}

String ReportManager::GenerateMarkdownReport(const String& title, const String& content, const Vector<String>& sources, const ValueArray& metrics) {
	String timestamp = FormatTime(GetSysTime(), "YYYY-MM-DD HH:mm:ss");
	String filename_timestamp = FormatTime(GetSysTime(), "YYYYMMDD_HHmmss");
	
	String report;
	report << "# " << title << "\n\n";
	report << "**Generated on:** " << timestamp << "\n";
	
	if (sources.GetCount() > 0) {
		report << "\n## Sources\n";
		for (const String& s : sources) report << "- " << s << "\n";
	}
	
	if (metrics.GetCount() > 0) {
		report << "\n## Performance Metrics\n";
		report << "| Operation | Duration (ms) |\n";
		report << "| :--- | :--- |\n";
		for (const auto& m : metrics) {
			if (m.Is<ValueMap>()) {
				ValueMap vm = m;
				report << "| " << (String)vm["operation"] << " | " << (String)vm["duration_ms"] << " |\n";
			}
		}
	}
	
	report << "\n## Content\n";
	report << content << "\n";
	
	String filename = "report_" + GetSafeTitle(title) + "_" + filename_timestamp + ".md";
	String path = AppendFileName(reports_dir, filename);
	
	SaveFile(path, report);
	return path;
}

String ReportManager::GenerateHtmlReport(const String& title, const String& content, const Vector<String>& sources, const ValueArray& metrics) {
	String timestamp = FormatTime(GetSysTime(), "YYYY-MM-DD HH:mm:ss");
	String filename_timestamp = FormatTime(GetSysTime(), "YYYYMMDD_HHmmss");
	
	String sources_html;
	if (sources.GetCount() > 0) {
		sources_html << "<h3>Sources</h3><ul>";
		for (const String& s : sources) sources_html << "<li>" << s << "</li>";
		sources_html << "</ul>";
	}
	
	String metrics_html;
	if (metrics.GetCount() > 0) {
		metrics_html << "<h3>Performance Metrics</h3><table><thead><tr><th>Operation</th><th>Duration (ms)</th></tr></thead><tbody>";
		for (const auto& m : metrics) {
			if (m.Is<ValueMap>()) {
				ValueMap vm = m;
				metrics_html << "<tr><td>" << (String)vm["operation"] << "</td><td>" << (String)vm["duration_ms"] << "</td></tr>";
			}
		}
		metrics_html << "</tbody></table>";
	}
	
	String html = Format(R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%s</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 40px 20px;
            background-color: #f9f9f9;
        }
        .report-container {
            background-color: #fff;
            padding: 40px;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        h1 {
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
        }
        .metadata {
            color: #7f8c8d;
            font-size: 0.9em;
            margin-bottom: 5px;
        }
        .sources, .metrics {
            margin-top: 20px;
            margin-bottom: 30px;
            padding: 15px;
            background-color: #ecf0f1;
            border-radius: 4px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            text-align: left;
            padding: 8px;
            border-bottom: 1px solid #ddd;
        }
        .content {
            white-space: pre-wrap;
            background-color: #fff;
            padding: 20px;
            border: 1px solid #eee;
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <div class="report-container">
        <h1>%s</h1>
        <p class="metadata">Generated on: %s</p>
        <div class="sources">
            %s
        </div>
        <div class="metrics">
            %s
        </div>
        <div class="content">%s</div>
    </div>
</body>
</html>)", title, title, timestamp, sources_html, metrics_html, content);
	
	String filename = "report_" + GetSafeTitle(title) + "_" + filename_timestamp + ".html";
	String path = AppendFileName(reports_dir, filename);
	
	SaveFile(path, html);
	return path;
}

END_UPP_NAMESPACE