#include "Aria.h"

NAMESPACE_UPP

const char* DuunitoriScraper::URL = "https://duunitori.fi/tyopaikat";

DuunitoriScraper::DuunitoriScraper(AriaNavigator& navigator, SiteManager& sm, const String& output_dir)
	: navigator(navigator)
	, sm(sm)
	, site_name("duunitori")
	, output_dir(output_dir)
{
	if (this->output_dir.IsEmpty()) {
		Date d = GetSysDate();
		String default_work = GetHomeDirFile(Format("Dev/Manager/99-work/%d/%02d", d.year, d.month));
		if (DirectoryExists(default_work) || DirectoryExists(GetHomeDirFile(Format("Dev/Manager/99-work/%d", d.year)))) {
			RealizeDirectory(default_work);
			this->output_dir = default_work;
		}
		else
			this->output_dir = sm.GetSiteDir(site_name);
	}
}

bool DuunitoriScraper::Navigate(const String& area) {
	String target_url = URL;
	if (!area.IsEmpty() && area != "all")
		target_url << "?alue=" << UrlEncode(area);

	try {
		String cur = navigator.GetCurrentURL();
		if (cur.Find("duunitori.fi") >= 0) {
			GetAriaLogger(site_name).Info("Already on Duunitori domain.");
			return true;
		}
	} catch (...) {}

	GetAriaLogger(site_name).Info("Navigating to Duunitori: " + target_url);
	try {
		navigator.Navigate(target_url);
		Sleep(2000);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger(site_name).Error("Failed to navigate: " + e);
		return false;
	}
}

bool DuunitoriScraper::Scrape(const String& area, const String& query) {
	if (!Navigate(area))
		return false;

	GetAriaLogger(site_name).Info("Starting Duunitori job scraping...");
	RealizeDirectory(output_dir);

	String script = R"(
		return (async function() {
			const area = )" + AsJSON(area) + R"(;
			const query = )" + AsJSON(query) + R"(;

			function buildUrl(page) {
				const params = new URLSearchParams();
				if (area && area !== 'all' && area !== '') params.set('alue', area);
				if (query && query.trim() !== '') params.set('haku', query.trim());
				if (page && page > 1) params.set('sivu', page);
				const str = params.toString();
				return 'https://duunitori.fi/tyopaikat' + (str ? '?' + str : '');
			}

			function parseJobBoxes(doc) {
				const boxes = Array.from(doc.querySelectorAll('.job-box'));
				return boxes.map(box => {
					const link = box.querySelector('a.job-box__hover') || box.querySelector('a[href*="/tyopaikat/tyo/"]');
					const titleEl = box.querySelector('.job-box__title') || link;
					const title = titleEl ? titleEl.innerText.trim() : 'Nimetön';
					let href = link ? link.getAttribute('href') : '';
					if (href && href.startsWith('/')) href = 'https://duunitori.fi' + href;
					const company = link ? (link.getAttribute('data-company') || '') : '';
					const category = link ? (link.getAttribute('data-category') || '') : '';
					const id = link ? (link.getAttribute('data-id') || '') : (href || title);
					const salaryEl = box.querySelector('.tag--salary');
					const salary = salaryEl ? salaryEl.innerText.trim() : '';
					const locEl = box.querySelector('.job-box__job-location');
					let location = locEl ? locEl.innerText.trim().replace(/[–\-\s]+$/, '') : '';
					const postedEl = box.querySelector('.job-box__job-posted');
					let posted = postedEl ? postedEl.innerText.trim() : '';
					const easyApply = !!box.querySelector('.tag--easy-apply');

					return {
						id,
						title,
						company,
						location,
						posted,
						salary,
						category,
						url: href,
						easyApply
					};
				});
			}

			// Step 1: Fetch page 1 to discover pagination
			const initialUrl = buildUrl(1);
			const res1 = await fetch(initialUrl);
			if (!res1.ok) {
				return { error: 'Failed to fetch page 1: ' + res1.status };
			}
			const text1 = await res1.text();
			const parser = new DOMParser();
			const doc1 = parser.parseFromString(text1, 'text/html');

			const pageNums = Array.from(doc1.querySelectorAll('.pagination__pagenum'))
				.map(p => parseInt(p.innerText.trim()))
				.filter(n => !isNaN(n));
			const maxPage = pageNums.length > 0 ? Math.max(...pageNums) : 1;

			let allJobs = parseJobBoxes(doc1);
			const seenIds = new Set(allJobs.map(j => j.id));

			// Step 2: Fetch subsequent pages in batches of 5
			const batchSize = 5;
			for (let p = 2; p <= maxPage; p += batchSize) {
				const batchPages = [];
				for (let b = 0; b < batchSize && (p + b) <= maxPage; b++) {
					batchPages.push(p + b);
				}

				const batchResults = await Promise.all(batchPages.map(async pageNum => {
					try {
						const r = await fetch(buildUrl(pageNum));
						if (!r.ok) return [];
						const html = await r.text();
						const doc = parser.parseFromString(html, 'text/html');
						return parseJobBoxes(doc);
					} catch(e) {
						return [];
					}
				}));

				for (const pageJobs of batchResults) {
					for (const job of pageJobs) {
						if (!seenIds.has(job.id)) {
							seenIds.add(job.id);
							allJobs.push(job);
						}
					}
				}

				// Small breath between batches
				await new Promise(res => setTimeout(res, 50));
			}

			function formatCard(job) {
				let text = job.title + '\n\n';
				text += '    ' + job.company + '\n';
				if (job.posted) text += '    ' + job.posted + '\n';
				text += '\n';
				if (job.location) text += job.location + '\n';
				if (job.salary) text += 'Palkka: ' + job.salary + '\n';
				text += '\n';
				if (job.category) text += '    ' + job.category + '\n';
				if (job.easyApply) text += '    Hae helposti\n';
				if (job.url) text += '    ' + job.url + '\n';
				return text;
			}

			const allFormatted = allJobs.map(formatCard).join('\n');

			const recKeywords = [
				'kehittäjä', 'ohjelm', 'developer', 'software', 'architect', 'arkkitehti',
				'data', 'it-', 'it ', 'tieto', 'järjestelmä', 'cloud', 'pilvi', 'devops',
				'suunnitteli', 'asiantuntija', 'projektipäällikkö', 'project', 'koordinaattori',
				'päällikkö', 'manager', 'toimisto', 'konsultti', 'consultant', 'musiik',
				'tuotta', 'kulttuur', 'ääni', 'media', 'teknologia', 'research', 'tutkija',
				'opettaja', 'kouluttaja', 'test'
			];

			const recJobs = allJobs.filter(j => {
				const t = (j.title || '').toLowerCase();
				const c = (j.category || '').toLowerCase();
				return recKeywords.some(k => t.includes(k) || c.includes(k));
			});

			const recFormatted = recJobs.map(formatCard).join('\n');

			return {
				total: allJobs.length,
				maxPage: maxPage,
				allText: allFormatted,
				recCount: recJobs.length,
				recText: recFormatted,
				jobs: allJobs
			};
		})();
	)";

	try {
		Value result_val = navigator.Eval(script);
		Value parsed = result_val.Is<String>() ? ParseJSON((String)result_val) : result_val;
		if (!parsed.Is<ValueMap>()) {
			GetAriaLogger(site_name).Error("Invalid response structure from scraping evaluation.");
			return false;
		}

		ValueMap root = parsed;
		if (root.Find("error") >= 0) {
			String err = root["error"];
			GetAriaLogger(site_name).Error("Scraping error: " + err);
			Cout() << "Scraping failed: " << err << "\n";
			return false;
		}

		int total = root["total"];
		int max_page = root["maxPage"];
		int rec_count = root["recCount"];
		String all_text = root["allText"];
		String rec_text = root["recText"];
		Value jobs_val = root["jobs"];

		String all_file = AppendFileName(output_dir, "kaikki-duunitori.txt");
		String json_file = AppendFileName(output_dir, "duunitori.json");
		String rec_file = AppendFileName(output_dir, "suositukset-duunitori.txt");

		if (!SaveFile(all_file, all_text)) {
			GetAriaLogger(site_name).Error("Failed to write: " + all_file);
			return false;
		}

		if (!SaveFile(json_file, AsJSON(jobs_val, true))) {
			GetAriaLogger(site_name).Error("Failed to write: " + json_file);
			return false;
		}

		if (!SaveFile(rec_file, rec_text)) {
			GetAriaLogger(site_name).Error("Failed to write: " + rec_file);
			return false;
		}

		sm.SetSiteData(site_name, "jobs", jobs_val);

		Cout() << "Scraped " << total << " jobs across " << max_page << " pages from Duunitori.\n";
		Cout() << "Saved results to directory: " << output_dir << "\n";
		Cout() << "  - " << all_file << " (" << total << " listings)\n";
		Cout() << "  - " << json_file << "\n";
		Cout() << "  - " << rec_file << " (" << rec_count << " recommendations)\n";

		GetAriaLogger(site_name).Info(Format("Successfully scraped %d jobs (recs: %d) into %s", total, rec_count, output_dir));
		return true;
	} catch (const Exc& e) {
		GetAriaLogger(site_name).Error("Error executing scraping script: " + e);
		Cout() << "Scraping failed: " << e << "\n";
		return false;
	}
}

END_UPP_NAMESPACE
