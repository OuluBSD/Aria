#include "Aria.h"

NAMESPACE_UPP

const char* TyomarkkinatoriScraper::URL = "https://tyomarkkinatori.fi/henkiloasiakkaat/avoimet-tyopaikat";

TyomarkkinatoriScraper::TyomarkkinatoriScraper(AriaNavigator& navigator, SiteManager& sm, const String& output_dir)
	: navigator(navigator)
	, sm(sm)
	, site_name("tyomarkkinatori")
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

bool TyomarkkinatoriScraper::Navigate() {
	try {
		String cur = navigator.GetCurrentURL();
		if (cur.Find("tyomarkkinatori.fi") >= 0) {
			GetAriaLogger(site_name).Info("Already on Työmarkkinatori page.");
			return true;
		}
	} catch (...) {}

	GetAriaLogger(site_name).Info("Navigating to Työmarkkinatori...");
	try {
		navigator.Navigate(URL);
		Sleep(2000);
		return true;
	} catch (const Exc& e) {
		GetAriaLogger(site_name).Error("Failed to navigate: " + e);
		return false;
	}
}

bool TyomarkkinatoriScraper::Scrape(const String& municipality_code, const String& query) {
	if (!Navigate())
		return false;

	GetAriaLogger(site_name).Info("Starting Työmarkkinatori job scraping...");
	RealizeDirectory(output_dir);

	String script = R"(
		return (async function() {
			async function getCodes(name) {
				try {
					const today = new Date().toISOString().slice(0, 10);
					const r = await fetch('https://tyomarkkinatori.fi/api/codes/v1/kopa/' + encodeURIComponent(name) + '/koodit?voimassa=' + today);
					const list = await r.json();
					const map = {};
					list.forEach(item => {
						const fi = item.selite ? item.selite.find(s => s.kielikoodi === 'fi') : null;
						map[item.tunnus] = fi ? fi.teksti : (item.selite && item.selite[0] ? item.selite[0].teksti : '');
					});
					return map;
				} catch(e) {
					return {};
				}
			}

			const codeMaps = {
				jatkuvuus: await getCodes('TYÖN_JATKUVUUS'),
				suhde: await getCodes('PALVELUSSUHDE'),
				aika: await getCodes('TYÖAIKA')
			};

			if (!codeMaps.aika['01']) {
				codeMaps.aika = {'01':'Kokoaikatyö','02':'Osa-aikatyö','0201':'Tuntia viikossa','0202':'Tuntia kuukaudessa'};
				codeMaps.jatkuvuus = {'01':'Toistaiseksi voimassa','02':'Määräaikainen','0201':'Kausityö','0202':'Kesätyö','0203':'Palkallinen harjoittelu'};
				codeMaps.suhde = {'01':'Työsuhde','0101':'Vuokratyö','0104':'Oppisopimus','02':'Virkasuhde','03':'Toimeksianto','04':'Yrittäjyys'};
			}

			const munic = )" + AsJSON(municipality_code) + R"(;
			const q = )" + AsJSON(query) + R"(;

			const filters = {};
			if (munic && munic !== 'all' && munic !== '') {
				filters.municipalities = [munic];
			}

			let allJobs = [];
			let page = 0;
			let total = 0;
			const pageSize = 90;

			while(true) {
				const body = {
					query: q,
					filters: filters,
					paging: { pageNumber: page, pageSize: pageSize },
					sorting: 'LATEST'
				};

				const res = await fetch('https://tyomarkkinatori.fi/api/jobpostingfulltext/search/v2/search', {
					method: 'POST',
					headers: { 'Content-Type': 'application/json' },
					body: JSON.stringify(body)
				});

				if (!res.ok) {
					break;
				}

				const data = await res.json();
				total = data.totalElements || 0;
				const list = data.content || [];
				if (list.length === 0) break;

				allJobs = allJobs.concat(list);
				if (allJobs.length >= total) break;
				page++;
			}

			function formatJob(job) {
				const title = job.title ? (job.title.fi || job.title.en || job.title.sv || 'Nimetön') : 'Nimetön';
				const employer = job.employer ? (job.employer.name || (job.employer.ownerName ? job.employer.ownerName.fi : '') || '') : '';
				
				let pubStr = '';
				if (job.publishDate) {
					const d = new Date(job.publishDate);
					pubStr = d.getDate() + '.' + (d.getMonth() + 1) + '.' + d.getFullYear();
				}
				
				let locStr = '';
				const munics = job.location && job.location.municipalities ? job.location.municipalities : [];
				if (munics.length === 1) {
					locStr = munics[0].label ? munics[0].label.fi : '';
				} else if (munics.length > 1) {
					const first = munics[0].label ? munics[0].label.fi : '';
					const others = munics.length - 1;
					locStr = first + ' + ' + others + ' ' + (others === 1 ? 'muu' : 'muuta');
				}
				
				let appEndStr = '';
				if (job.applicationPeriodEndDate) {
					const d = new Date(job.applicationPeriodEndDate);
					appEndStr = 'Hakuaika päättyy ' + d.getDate() + '.' + (d.getMonth() + 1) + '.' + d.getFullYear();
				}
				
				const terms = [];
				const relCode = job.employmentRelationships || '';
				if (codeMaps.suhde[relCode]) terms.push(codeMaps.suhde[relCode]);
				
				const contCodes = job.continuityOfWork || [];
				contCodes.forEach(c => { if (codeMaps.jatkuvuus[c]) terms.push(codeMaps.jatkuvuus[c]); });
				
				const workTimeCode = job.workTime || '';
				if (codeMaps.aika[workTimeCode]) terms.push(codeMaps.aika[workTimeCode]);
				
				let text = title + '\n\n';
				text += '    ' + employer + '\n';
				if (pubStr) text += '    Julkaistu ' + pubStr + '\n';
				text += '\n';
				if (locStr) text += locStr + '\n';
				if (appEndStr) text += appEndStr + '\n';
				text += '\n';
				terms.forEach(t => { text += '    ' + t + '\n'; });
				return text;
			}

			const allFormatted = allJobs.map(formatJob).join('\n');

			const recKeywords = [
				'kehittäjä', 'ohjelm', 'developer', 'software', 'architect', 'arkkitehti',
				'data', 'it-', 'it ', 'tieto', 'järjestelmä', 'cloud', 'pilvi', 'devops',
				'suunnitteli', 'asiantuntija', 'projektipäällikkö', 'project', 'koordinaattori',
				'päällikkö', 'manager', 'toimisto', 'konsultti', 'consultant', 'musiik',
				'tuotta', 'kulttuur', 'ääni', 'media', 'teknologia', 'research', 'tutkija',
				'opettaja', 'kouluttaja', 'test'
			];

			const recJobs = allJobs.filter(j => {
				const t = (j.title?.fi || '').toLowerCase();
				return recKeywords.some(k => t.includes(k));
			});

			const recFormatted = recJobs.map(formatJob).join('\n');

			return {
				total: total,
				fetched: allJobs.length,
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
		int total = root["total"];
		int fetched = root["fetched"];
		int rec_count = root["recCount"];
		String all_text = root["allText"];
		String rec_text = root["recText"];
		Value jobs_val = root["jobs"];

		String all_file = AppendFileName(output_dir, "kaikki-tyomarkkinatori.txt");
		String json_file = AppendFileName(output_dir, "tyomarkkinatori.json");
		String rec_file = AppendFileName(output_dir, "suositukset.txt");

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

		Cout() << "Scraped " << fetched << " / " << total << " jobs from Työmarkkinatori.\n";
		Cout() << "Saved results to directory: " << output_dir << "\n";
		Cout() << "  - " << all_file << " (" << fetched << " listings)\n";
		Cout() << "  - " << json_file << "\n";
		Cout() << "  - " << rec_file << " (" << rec_count << " recommendations)\n";

		GetAriaLogger(site_name).Info(Format("Successfully scraped %d jobs (recs: %d) into %s", fetched, rec_count, output_dir));
		return true;
	} catch (const Exc& e) {
		GetAriaLogger(site_name).Error("Error executing scraping script: " + e);
		Cout() << "Scraping failed: " << e << "\n";
		return false;
	}
}

END_UPP_NAMESPACE
