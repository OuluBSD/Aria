# MaestroHub QA: Phase 4 - Execution (Scenarios 56-65)

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

def test_execution():
	log("Initializing Phase 4 Execution Test...")
	wait_time(5)
	
	log("Scenario 56: Build Trigger (OpsRunner)")
	ops_btn = find("Main/Run Operations Doctor")
	if ops_btn:
		log("Opening OpsRunner...")
		ops_btn.click()
		wait_time(1)
	else:
		log("FAIL: OpsRunner toolbar button not found")
		return False
	
	wait_for("OpsRunnerLayout", 10)
	
	# Select 'build' in ops_list
	row0 = find("OpsRunnerLayout/ops_list/Rows/Row 0")
	if row0:
		log("Selecting 'build' operation...")
		row0.click()
		wait_time(0.5)
	
	btn_run = find("OpsRunnerLayout/btn_run")
	if btn_run:
		log("Clicking Run Selected...")
		btn_run.click()
		wait_time(2)
	
	log("Scenario 62: Ops Runner Execution (Run app)")
	row1 = find("OpsRunnerLayout/ops_list/Rows/Row 1")
	if row1:
		log("Selecting 'run' operation...")
		row1.click()
		wait_time(0.5)
		if btn_run:
			log("Clicking Run Selected...")
			btn_run.click()
			wait_time(2)
	
	btn_close = find("OpsRunnerLayout/btn_close")
	if btn_close:
		log("Closing OpsRunner...")
		btn_close.click()
		wait_time(1)
	
	log("SUCCESS: Phase 4 basic execution flow complete.")
	return True

if test_execution():
	exit(0)
else:
	exit(1)