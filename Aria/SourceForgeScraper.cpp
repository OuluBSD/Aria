#include "Aria.h"
#include "SourceForgeScraper.h"

NAMESPACE_UPP

SourceForgeScraper::SourceForgeScraper(AriaNavigator& navigator, SiteManager& sm,
                                       const String& project, const String& output_dir)
	: navigator(navigator)
	, sm(sm)
	, site_name("sourceforge")
	, project(project.IsEmpty() ? "topside" : project)
	, output_dir(output_dir)
{
	if(this->output_dir.IsEmpty())
		this->output_dir = AppendFileName(sm.GetSiteDir(site_name), this->project);
	RealizeDirectory(this->output_dir);
}

void SourceForgeScraper::SetProject(const String& new_project)
{
	project = new_project;
	output_dir = AppendFileName(sm.GetSiteDir(site_name), project);
	RealizeDirectory(output_dir);
}

bool SourceForgeScraper::EnsureTab(const String& target_url)
{
	GetAriaLogger(site_name).Info("SourceForge navigating to: " + target_url);

	try {
		navigator.ConnectToSession("firefox");
		ValueArray tabs = navigator.ListTabs();
		bool switched = false;
		for(int i = 0; i < tabs.GetCount(); i++) {
			ValueMap m = tabs[i];
			String u = m["url"];
			if(u.Find("sourceforge.net/projects/" + project) >= 0 || u.Find("sourceforge.net/p/" + project) >= 0) {
				navigator.GotoTab(m["handle"]);
				switched = true;
				break;
			}
		}
		navigator.Navigate(target_url);
		Sleep(1500); // Wait for SourceForge DOM/scripts
		return true;
	} catch(const Exc& e) {
		GetAriaLogger(site_name).Error("EnsureTab failed: " + e);
		return false;
	}
}

ValueMap SourceForgeScraper::ScrapeSummary()
{
	ValueMap out;
	String url = "https://sourceforge.net/projects/" + project + "/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = (document.querySelector('.project-title, h1.title, #project-header h1, h1') || {}).innerText || document.title;\n"
		"let desc = (document.querySelector('#project-description, .description') || {}).innerText || '';\n"
		"let features = Array.from(document.querySelectorAll('.features-list li, section.features li')).map(li => li.innerText.trim()).filter(Boolean);\n"
		"let navItems = Array.from(document.querySelectorAll('#top_nav li a, .navigation li a, ul.nav li a, nav a')).map(a => ({\n"
		"  text: a.innerText.trim(),\n"
		"  href: a.href\n"
		"})).filter(item => item.text.length > 0);\n"
		"let details = (document.querySelector('#additional-details, .project-info') || {}).innerText || '';\n"
		"return JSON.stringify({\n"
		"  project: '" + project + "',\n"
		"  title: title.trim(),\n"
		"  description: desc.trim(),\n"
		"  features: features,\n"
		"  nav_items: navItems,\n"
		"  details: details.replace(/\\n+/g, ' ').trim(),\n"
		"  url: window.location.href\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeSummary error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "summary.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeFiles()
{
	ValueMap out;
	String url = "https://sourceforge.net/projects/" + project + "/files/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let rows = Array.from(document.querySelectorAll('#files_list tbody tr, table.files-list tr')).map(r => {\n"
		"  let nameEl = r.querySelector('th a, td.name a');\n"
		"  let sizeEl = r.querySelector('td.size, td:nth-child(2)');\n"
		"  let dateEl = r.querySelector('td.date, td:nth-child(3)');\n"
		"  let isDir = !!r.querySelector('.folder, .icon-folder');\n"
		"  return {\n"
		"    name: nameEl ? nameEl.innerText.trim() : '',\n"
		"    url: nameEl ? nameEl.href : '',\n"
		"    size: sizeEl ? sizeEl.innerText.trim() : '',\n"
		"    date: dateEl ? dateEl.innerText.trim() : '',\n"
		"    is_dir: isDir\n"
		"  };\n"
		"}).filter(f => f.name.length > 0);\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  file_count: rows.length,\n"
		"  files: rows,\n"
		"  url: window.location.href\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeFiles error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "files.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeReviews()
{
	ValueMap out;
	String url = "https://sourceforge.net/projects/" + project + "/reviews/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let rating = (document.querySelector('.rating-average, .rating') || {}).innerText || '0';\n"
		"let reviews = Array.from(document.querySelectorAll('.review-item, .user-review')).map(r => ({\n"
		"  text: r.innerText.replace(/\\n+/g, ' ').trim()\n"
		"}));\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  rating: rating.trim(),\n"
		"  review_count: reviews.length,\n"
		"  reviews: reviews\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeReviews error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "reviews.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeSupport()
{
	ValueMap out;
	String url = "https://sourceforge.net/projects/" + project + "/support";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let sections = Array.from(document.querySelectorAll('section, .section, #content h3, #content h2')).map(s => s.innerText.trim()).filter(Boolean);\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  sections: sections,\n"
		"  url: window.location.href\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeSupport error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "support.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeBlog()
{
	ValueMap out;
	String url = "https://sourceforge.net/p/" + project + "/blog/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let posts = Array.from(document.querySelectorAll('.blog_post, .post, article')).map(p => {\n"
		"  let tEl = p.querySelector('h2, h3, .title');\n"
		"  let bodyEl = p.querySelector('.markdown_content, .content, p');\n"
		"  return {\n"
		"    title: tEl ? tEl.innerText.trim() : '',\n"
		"    content: bodyEl ? bodyEl.innerText.replace(/\\n+/g, ' ').trim() : ''\n"
		"  };\n"
		"});\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  post_count: posts.length,\n"
		"  posts: posts\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeBlog error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "blog.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeCode(const String& branch)
{
	ValueMap out;
	String b = branch.IsEmpty() ? "stable" : branch;
	String url = "https://sourceforge.net/p/" + project + "/code/ci/" + b + "/tree/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let commitHashEl = document.querySelector('h2.dark, .commit-details, a[href*=\"/ci/\"]');\n"
		"let commitHash = commitHashEl ? commitHashEl.innerText.trim() : '';\n"
		"let branches = Array.from(document.querySelectorAll('#branches a, .branch-list a, select option')).map(o => o.innerText.trim()).filter(Boolean);\n"
		"let rows = Array.from(document.querySelectorAll('table.file-browser tr, table tr')).map(r => {\n"
		"  let link = r.querySelector('a');\n"
		"  let isDir = !!r.querySelector('.icon-folder, [class*=\"folder\"]');\n"
		"  return {\n"
		"    name: link ? link.innerText.trim() : '',\n"
		"    href: link ? link.href : '',\n"
		"    is_dir: isDir\n"
		"  };\n"
		"}).filter(f => f.name.length > 0 && !f.name.startsWith('Download') && !f.name.startsWith('History'));\n"
		"let readmeEl = document.querySelector('.markdown_content, #readme');\n"
		"let readme = readmeEl ? readmeEl.innerText.slice(0, 2000) : '';\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  branch: '" + b + "',\n"
		"  commit_hash: commitHash,\n"
		"  branches: branches,\n"
		"  file_count: rows.length,\n"
		"  files: rows,\n"
		"  readme_snippet: readme\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeCode error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "code.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeWiki()
{
	ValueMap out;
	String url = "https://sourceforge.net/p/" + project + "/wiki/Home/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let bodyEl = document.querySelector('#content .markdown_content, #content, main');\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  content: bodyEl ? bodyEl.innerText.slice(0, 2500).replace(/\\n+/g, ' ').trim() : '',\n"
		"  url: window.location.href\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeWiki error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "wiki.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeTickets()
{
	ValueMap out;
	String url = "https://sourceforge.net/p/" + project + "/tickets/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let tickets = Array.from(document.querySelectorAll('table.ticket-list tr, tr.ticket')).map(r => ({\n"
		"  text: r.innerText.replace(/\\n+/g, ' ').trim()\n"
		"}));\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  ticket_count: tickets.length,\n"
		"  tickets: tickets\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeTickets error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "tickets.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeDiscussion()
{
	ValueMap out;
	String url = "https://sourceforge.net/p/" + project + "/discussion/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let forums = Array.from(document.querySelectorAll('.forum, table tr')).map(f => ({\n"
		"  text: f.innerText.replace(/\\n+/g, ' ').trim()\n"
		"}));\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  forum_count: forums.length,\n"
		"  forums: forums\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeDiscussion error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "discussion.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeMailman()
{
	ValueMap out;
	String url = "https://sourceforge.net/p/" + project + "/mailman/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let lists = Array.from(document.querySelectorAll('table tr, ul li')).map(li => ({\n"
		"  text: li.innerText.replace(/\\n+/g, ' ').trim()\n"
		"}));\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  lists: lists\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeMailman error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "mailman.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeAdmin()
{
	ValueMap out;
	String url = "https://sourceforge.net/p/" + project + "/admin/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let tools = Array.from(document.querySelectorAll('#tools_list li, .tools a, ul.admin-nav li')).map(t => t.innerText.trim()).filter(Boolean);\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  tools: tools,\n"
		"  url: window.location.href\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeAdmin error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "admin.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeSubproject(const String& subname)
{
	ValueMap out;
	String s = subname.IsEmpty() ? "subproject" : subname;
	String url = "https://sourceforge.net/projects/" + s + "." + project + ".p/";
	if(!EnsureTab(url)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let title = document.title;\n"
		"let headings = Array.from(document.querySelectorAll('h1, h2, h3')).map(h => h.innerText.trim()).filter(Boolean);\n"
		"let snippet = (document.querySelector('#content, main') || document.body).innerText.slice(0, 600).replace(/\\n+/g, ' ').trim();\n"
		"return JSON.stringify({\n"
		"  title: title,\n"
		"  subproject: '" + s + "',\n"
		"  headings: headings,\n"
		"  snippet: snippet\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeSubproject error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "subproject.json"), AsJSON(out, true));
	return out;
}

ValueMap SourceForgeScraper::ScrapeAll()
{
	ValueMap all;
	Cout() << "Scraping SourceForge Summary for " << project << "...\n";
	all.Set("summary", ScrapeSummary());

	Cout() << "Scraping Files...\n";
	all.Set("files", ScrapeFiles());

	Cout() << "Scraping Reviews...\n";
	all.Set("reviews", ScrapeReviews());

	Cout() << "Scraping Support...\n";
	all.Set("support", ScrapeSupport());

	Cout() << "Scraping Blog...\n";
	all.Set("blog", ScrapeBlog());

	Cout() << "Scraping Code...\n";
	all.Set("code", ScrapeCode());

	Cout() << "Scraping Wiki...\n";
	all.Set("wiki", ScrapeWiki());

	Cout() << "Scraping Tickets...\n";
	all.Set("tickets", ScrapeTickets());

	Cout() << "Scraping Discussion...\n";
	all.Set("discussion", ScrapeDiscussion());

	Cout() << "Scraping Mailman...\n";
	all.Set("mailman", ScrapeMailman());

	Cout() << "Scraping Admin...\n";
	all.Set("admin", ScrapeAdmin());

	Cout() << "Scraping Subproject...\n";
	all.Set("subproject", ScrapeSubproject());

	SaveFile(AppendFileName(output_dir, "full_topside_scrape.json"), AsJSON(all, true));
	Cout() << "SourceForge scrape complete! Saved to " << output_dir << "\n";
	return all;
}

bool SourceForgeScraper::RunSelfTests()
{
	Cout() << "[SourceForgeScraper Test] Starting verification against " << project << "...\n";
	ValueMap sum = ScrapeSummary();
	if(sum.Find("title") < 0) {
		Cout() << "[SourceForgeScraper Test] FAILED: summary missing\n";
		return false;
	}
	Cout() << "[SourceForgeScraper Test] Summary OK: " << (String)sum["title"] << "\n";

	ValueMap code = ScrapeCode("stable");
	if(code.Find("title") < 0) {
		Cout() << "[SourceForgeScraper Test] FAILED: code missing\n";
		return false;
	}
	Cout() << "[SourceForgeScraper Test] Code OK: branch=" << (String)code["branch"] << "\n";

	ValueMap blog = ScrapeBlog();
	Cout() << "[SourceForgeScraper Test] Blog OK: posts=" << (int)blog["post_count"] << "\n";

	Cout() << "[SourceForgeScraper Test] ALL TESTS PASSED!\n";
	return true;
}

END_UPP_NAMESPACE
