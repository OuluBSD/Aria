# MaestroHub QA: Phase 2 - Planning & Architecture (Scenarios 16-20)

def wait_for(path, count):
	log("Waiting for: " + path)
	i = 0
	while i < count:
		el = find(path)
		if el:
			log("Found: " + path)
			return el
		wait_time(0.5)
		i = i + 1
	log("TIMEOUT: " + path)
	return None

def test_planning():
	log("Initializing Phase 2 Planning Test...")
	wait_time(5)
	
	log("Scenario 16: Requirement Creation (Issue)")
	issue_tab = wait_for("Main/MainTabs/Issue Tracker", 10)
	if issue_tab:
		log("Clicking Issue Tracker tab...")
		issue_tab.click()
		wait_time(1)
	else:
		log("FAIL: Issue Tracker tab not found")
		return False
	
	# Open Create Issue dialog via menu
	create_menu = find("Main/Main/Issues/Create Issue...")
	if create_menu:
		log("Clicking Create Issue menu...")
		create_menu.click()
	else:
		log("FAIL: Create Issue menu not found")
		return False
	
	wait_for("IssueCreateLayout", 10)
	title_edit = find("IssueCreateLayout/title")
	if title_edit:
		title_edit.set("General Ledger Implementation")
	
	desc_edit = find("IssueCreateLayout/description")
	if desc_edit:
		desc_edit.set("Implement the core accounting ledger with transaction support.")
	
	btn_ok = find("IssueCreateLayout/ok")
	if btn_ok:
		log("Clicking Create button...")
		btn_ok.click()
		wait_time(1)
	
	log("Scenario 19: Playbook Design")
	play_tab = find("Main/MainTabs/Strategy Playbooks")
	if play_tab:
		log("Clicking Strategy Playbooks tab...")
		play_tab.click()
		wait_time(1)
	
	btn_new = find("Main/MainTabs/PlaybookLayout/toolbar/New")
	if btn_new:
		log("Clicking New Playbook...")
		btn_new.click()
		wait_time(1)
	
	id_edit = find("Main/MainTabs/PlaybookLayout/playbook_id/playbook_id")
	if id_edit:
		id_edit.set("ledger_scaffolding")
	
	title_edit = find("Main/MainTabs/PlaybookLayout/title/title")
	if title_edit:
		title_edit.set("Ledger Scaffolding Strategy")
	
	log("Scenario 20: Visual Logic Entry")
	vis_tab = find("Main/MainTabs/PlaybookLayout/split/detail_tabs/Visual Logic")
	if vis_tab:
		log("Switching to Visual Logic tab...")
		vis_tab.click()
		wait_time(1)
	
	log("SUCCESS: Phase 2 basic planning complete.")
	return True

if test_planning():
	exit(0)
else:
	exit(1)