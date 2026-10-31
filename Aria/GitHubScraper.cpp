#include "Aria.h"
#include "GitHubScraper.h"

NAMESPACE_UPP

GitHubScraper::GitHubScraper(AriaNavigator& navigator, SiteManager& sm,
                             const String& owner, const String& repo,
                             const String& output_dir)
	: navigator(navigator)
	, sm(sm)
	, site_name("github")
	, owner(owner.IsEmpty() ? "SeppoPakonen" : owner)
	, repo(repo.IsEmpty() ? "ai-upp-priv" : repo)
	, output_dir(output_dir)
{
	if(this->output_dir.IsEmpty())
		this->output_dir = AppendFileName(sm.GetSiteDir(site_name), this->owner + "_" + this->repo);
	RealizeDirectory(this->output_dir);
}

void GitHubScraper::SetTarget(const String& new_owner, const String& new_repo)
{
	owner = new_owner;
	repo = new_repo;
	output_dir = AppendFileName(sm.GetSiteDir(site_name), owner + "_" + repo);
	RealizeDirectory(output_dir);
}

bool GitHubScraper::EnsureRepoTab(const String& path)
{
	String target_url = "https://github.com/" + owner + "/" + repo;
	if(!path.IsEmpty()) {
		if(!path.StartsWith("/") && !path.StartsWith("?"))
			target_url << "/";
		target_url << path;
	}

	GetAriaLogger(site_name).Info("GitHub navigating to: " + target_url);

	try {
		navigator.ConnectToSession("firefox");
		// Try to switch to existing github tab
		ValueArray tabs = navigator.ListTabs();
		bool switched = false;
		for(int i = 0; i < tabs.GetCount(); i++) {
			ValueMap m = tabs[i];
			String u = m["url"];
			if(u.Find("github.com") >= 0) {
				navigator.GotoTab(m["handle"]);
				switched = true;
				break;
			}
		}
		if(!switched)
			navigator.Navigate(target_url);
		else
			navigator.Navigate(target_url);

		Sleep(1500); // Wait for React/Turbo render
		return true;
	} catch(const Exc& e) {
		GetAriaLogger(site_name).Error("EnsureRepoTab failed: " + e);
		return false;
	}
}

ValueMap GitHubScraper::ScrapeOverview()
{
	ValueMap out;
	if(!EnsureRepoTab("")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let isPrivate = !window.location.href.includes('public') && !!document.querySelector('.Label--secondary, [aria-label*=\"Private\"], [class*=\"private\"]');\n"
		"let branchEl = document.querySelector('#branch-select-menu summary span, button[id*=\"branch-select\"] span');\n"
		"let branch = branchEl ? branchEl.innerText.trim() : 'master';\n"
		"let descEl = document.querySelector('.f4.my-3, [data-testid=\"about-description\"]');\n"
		"let desc = descEl ? descEl.innerText.trim() : '';\n"
		"let fileRows = Array.from(document.querySelectorAll('table tbody tr.react-directory-row, table tbody tr, [aria-labelledby=\"files\"] div[role=\"row\"]'));\n"
		"let files = [];\n"
		"for (let r of fileRows) {\n"
		"  let nameEl = r.querySelector('a.Link--primary, a');\n"
		"  let isDir = !!r.querySelector('svg[aria-label=\"Directory\"], svg.octicon-file-directory, svg.octicon-file-directory-fill');\n"
		"  let msgEl = r.querySelector('a.Link--secondary, td:nth-child(3)');\n"
		"  let ageEl = r.querySelector('relative-time, td:last-child');\n"
		"  if (nameEl && nameEl.innerText.trim()) {\n"
		"    files.push({\n"
		"      name: nameEl.innerText.trim(),\n"
		"      type: isDir ? 'dir' : 'file',\n"
		"      commit_msg: msgEl ? msgEl.innerText.trim() : '',\n"
		"      age: ageEl ? ageEl.innerText.trim() : ''\n"
		"    });\n"
		"  }\n"
		"}\n"
		"let readmeEl = document.querySelector('#readme article, article.markdown-body');\n"
		"let readmeSnippet = readmeEl ? readmeEl.innerText.slice(0, 2000) : '';\n"
		"return JSON.stringify({\n"
		"  owner: '" + owner + "',\n"
		"  repo: '" + repo + "',\n"
		"  is_private: isPrivate,\n"
		"  branch: branch,\n"
		"  description: desc,\n"
		"  files: files,\n"
		"  file_count: files.length,\n"
		"  readme_snippet: readmeSnippet,\n"
		"  url: window.location.href,\n"
		"  title: document.title\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		Value parsed = v.Is<String>() ? ParseJSON((String)v) : v;
		if(parsed.Is<ValueMap>())
			out = parsed;
		else
			out.Set("raw", v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeOverview error: ") + e);
	}

	String path = AppendFileName(output_dir, "overview.json");
	SaveFile(path, AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeIssues()
{
	ValueMap out;
	if(!EnsureRepoTab("issues")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let items = Array.from(document.querySelectorAll('.js-issue-row, div[data-testid=\"issue-item\"]')).map(r => {\n"
		"  let titleEl = r.querySelector('a.IssueLabel, a.Link--primary');\n"
		"  let authorEl = r.querySelector('a.Link--muted, a[data-hovercard-type=\"user\"]');\n"
		"  return {\n"
		"    title: titleEl ? titleEl.innerText.trim() : '',\n"
		"    author: authorEl ? authorEl.innerText.trim() : '',\n"
		"    snippet: r.innerText.replace(/\\n+/g, ' ')\n"
		"  };\n"
		"});\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  count: items.length,\n"
		"  issues: items\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeIssues error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "issues.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapePulls()
{
	ValueMap out;
	if(!EnsureRepoTab("pulls")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let prs = Array.from(document.querySelectorAll('.js-issue-row, div[data-testid=\"pr-item\"]')).map(r => {\n"
		"  let titleEl = r.querySelector('a.Link--primary');\n"
		"  return {\n"
		"    title: titleEl ? titleEl.innerText.trim() : '',\n"
		"    snippet: r.innerText.replace(/\\n+/g, ' ')\n"
		"  };\n"
		"});\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  count: prs.length,\n"
		"  pull_requests: prs\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapePulls error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "pulls.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeActions()
{
	ValueMap out;
	if(!EnsureRepoTab("actions")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let runs = Array.from(document.querySelectorAll('.Box-row, [data-testid=\"workflow-run-item\"]')).map(r => ({\n"
		"  text: r.innerText.replace(/\\n+/g, ' ')\n"
		"}));\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  count: runs.length,\n"
		"  runs: runs\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeActions error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "actions.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeProjects()
{
	ValueMap out;
	if(!EnsureRepoTab("projects")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let items = Array.from(document.querySelectorAll('[data-testid=\"project-item\"], .Box-row')).map(r => ({\n"
		"  text: r.innerText.replace(/\\n+/g, ' ')\n"
		"}));\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  count: items.length,\n"
		"  projects: items\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeProjects error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "projects.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeSecurity()
{
	ValueMap out;
	if(!EnsureRepoTab("security")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let text = (document.querySelector('main') || document.body).innerText.replace(/\\n+/g, ' ').slice(0, 2000);\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  snippet: text\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeSecurity error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "security.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeDependencies()
{
	ValueMap out;
	if(!EnsureRepoTab("network/dependencies")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let text = (document.querySelector('main') || document.body).innerText.replace(/\\n+/g, ' ').slice(0, 2000);\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  snippet: text\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeDependencies error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "dependencies.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeSettings()
{
	ValueMap out;
	if(!EnsureRepoTab("settings")) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let repoNameInput = document.querySelector('input[name=\"new_name\"], input[id=\"rename-field\"]');\n"
		"let branchEl = document.querySelector('#default-branch-name, [data-testid=\"default-branch-name\"]');\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  repo_name: repoNameInput ? repoNameInput.value : '',\n"
		"  default_branch: branchEl ? branchEl.innerText.trim() : 'master'\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeSettings error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "settings.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeAgents(const String& author)
{
	ValueMap out;
	if(!EnsureRepoTab("agents?author=" + author)) {
		out.Set("error", "Navigation failed");
		return out;
	}

	String js =
		"let text = (document.querySelector('main') || document.body).innerText.replace(/\\n+/g, ' ').slice(0, 2000);\n"
		"return JSON.stringify({\n"
		"  title: document.title,\n"
		"  author: '" + author + "',\n"
		"  snippet: text\n"
		"});";

	try {
		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeAgents error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "agents.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::ScrapeUserProfile(const String& username)
{
	ValueMap out;
	String user_url = "https://github.com/" + username;
	try {
		navigator.ConnectToSession("firefox");
		navigator.Navigate(user_url);
		Sleep(1500);

		String js =
			"let nameEl = document.querySelector('.vcard-fullname');\n"
			"let userEl = document.querySelector('.vcard-username');\n"
			"let bioEl = document.querySelector('.user-profile-bio');\n"
			"let pinned = Array.from(document.querySelectorAll('.pinned-item-list-item-content')).map(p => {\n"
			"  let t = p.querySelector('.repo');\n"
			"  let d = p.querySelector('.pinned-item-desc');\n"
			"  let l = p.querySelector('[itemprop=\"programmingLanguage\"]');\n"
			"  return {\n"
			"    name: t ? t.innerText.trim() : '',\n"
			"    desc: d ? d.innerText.trim() : '',\n"
			"    lang: l ? l.innerText.trim() : ''\n"
			"  };\n"
			"});\n"
			"return JSON.stringify({\n"
			"  fullname: nameEl ? nameEl.innerText.trim() : '',\n"
			"  username: userEl ? userEl.innerText.trim() : '',\n"
			"  bio: bioEl ? bioEl.innerText.trim() : '',\n"
			"  pinned: pinned\n"
			"});";

		Value v = navigator.Eval(js);
		out = (v.Is<String>() ? ParseJSON((String)v) : v);
	} catch(const Exc& e) {
		out.Set("error", String("ScrapeUserProfile error: ") + e);
	}

	SaveFile(AppendFileName(output_dir, "user_profile.json"), AsJSON(out, true));
	return out;
}

ValueMap GitHubScraper::SyncWithGhCli()
{
	ValueMap res;
	res.Set("owner", owner);
	res.Set("repo", repo);

	// gh repo view
	String repo_cmd = "gh repo view " + owner + "/" + repo +
	                  " --json name,description,defaultBranchRef,isPrivate,url,stargazerCount,forkCount,watchers,visibility";
	String repo_out = Sys(repo_cmd);
	if(!repo_out.IsEmpty()) {
		Value v = ParseJSON(repo_out);
		if(v.Is<ValueMap>()) res.Set("repo_cli", v);
	}

	// gh issue list
	String issue_cmd = "gh issue list -R " + owner + "/" + repo +
	                   " --state all --limit 20 --json number,title,state,author,createdAt,updatedAt";
	String issue_out = Sys(issue_cmd);
	if(!issue_out.IsEmpty()) {
		Value v = ParseJSON(issue_out);
		res.Set("issues_cli", v);
	}

	// gh pr list
	String pr_cmd = "gh pr list -R " + owner + "/" + repo +
	                " --state all --limit 20 --json number,title,state,author,createdAt,updatedAt";
	String pr_out = Sys(pr_cmd);
	if(!pr_out.IsEmpty()) {
		Value v = ParseJSON(pr_out);
		res.Set("pulls_cli", v);
	}

	// gh run list
	String run_cmd = "gh run list -R " + owner + "/" + repo +
	                 " --limit 10 --json id,name,status,conclusion,headBranch";
	String run_out = Sys(run_cmd);
	if(!run_out.IsEmpty()) {
		Value v = ParseJSON(run_out);
		res.Set("runs_cli", v);
	}

	SaveFile(AppendFileName(output_dir, "gh_cli_sync.json"), AsJSON(res, true));
	return res;
}

ValueMap GitHubScraper::ScrapeAll()
{
	ValueMap all;
	Cout() << "Scraping GitHub Overview for " << owner << "/" << repo << "...\n";
	all.Set("overview", ScrapeOverview());

	Cout() << "Scraping Issues...\n";
	all.Set("issues", ScrapeIssues());

	Cout() << "Scraping Pull Requests...\n";
	all.Set("pulls", ScrapePulls());

	Cout() << "Scraping Actions...\n";
	all.Set("actions", ScrapeActions());

	Cout() << "Scraping Projects...\n";
	all.Set("projects", ScrapeProjects());

	Cout() << "Scraping Security...\n";
	all.Set("security", ScrapeSecurity());

	Cout() << "Scraping Dependencies...\n";
	all.Set("dependencies", ScrapeDependencies());

	Cout() << "Scraping Settings...\n";
	all.Set("settings", ScrapeSettings());

	Cout() << "Scraping SweAgent Tab (agents?author=" << owner << ")...\n";
	all.Set("agents", ScrapeAgents(owner));

	Cout() << "Scraping User Profile (" << owner << ")...\n";
	all.Set("profile", ScrapeUserProfile(owner));

	Cout() << "Syncing with gh CLI...\n";
	all.Set("cli", SyncWithGhCli());

	SaveFile(AppendFileName(output_dir, "summary.json"), AsJSON(all, true));
	Cout() << "GitHub scrape complete! Saved to " << output_dir << "\n";
	return all;
}

bool GitHubScraper::RunSelfTests()
{
	Cout() << "[GitHubScraper Test] Starting verification against " << owner << "/" << repo << "...\n";
	ValueMap ov = ScrapeOverview();
	if(ov.Find("files") < 0) {
		Cout() << "[GitHubScraper Test] FAILED: overview files missing\n";
		return false;
	}
	int files = ov["file_count"];
	Cout() << "[GitHubScraper Test] Overview OK, detected files: " << files << "\n";

	ValueMap cli = SyncWithGhCli();
	if(cli.Find("repo_cli") < 0) {
		Cout() << "[GitHubScraper Test] FAILED: gh cli sync missing repo_cli\n";
		return false;
	}
	Cout() << "[GitHubScraper Test] gh CLI sync OK\n";
	Cout() << "[GitHubScraper Test] ALL TESTS PASSED!\n";
	return true;
}

END_UPP_NAMESPACE

