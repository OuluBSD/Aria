# MaestroHub QA: Phase 6 - Validation & Evidence (Scenarios 91-100)

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

def test_validation():
	log("Initializing Phase 6 Validation Test...")
	wait_time(5)
	
	log("Scenario 94: Evidence Collection")
	evid_tab = wait_for("Main/MainTabs/Evidence Locker", 10)
	if evid_tab:
		log("Clicking Evidence Locker tab...")
		evid_tab.click()
		wait_time(1)
	
	# Verify evidence list exists
	evid_list = wait_for("Main/MainTabs/EvidenceLayout/main_split/evidence_list/row", 10)
	if evid_list:
		log("Found evidence list.")
	else:
		log("FAIL: Evidence list not found")
		return False
	
	log("Scenario 91: UX Evaluation Init")
	ux_menu = find("Main/Main/Product/UX Evaluation Factory...")
	if ux_menu:
		log("Opening UX Evaluation Factory...")
		ux_menu.click()
		wait_time(1)
	else:
		log("FAIL: UX Factory menu not found")
		return False
	
	# Path should be UXEvaluationFactory/test_list/row
	ux_list = wait_for("UXEvaluationFactory/test_list/row", 10)
	if ux_list:
		log("Found UX test list.")
		# Dismiss dialog
		send_key(K_ESCAPE)
		wait_time(1)
	else:
		log("FAIL: UX test list not found")
		return False
	
	log("SUCCESS: Phase 6 validation flow complete.")
	return True

if test_validation():
	exit(0)
else:
	exit(1)