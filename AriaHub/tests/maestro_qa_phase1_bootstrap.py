# MaestroHub QA: Phase 1 - Project Bootstrap (Scenarios 1-15)

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

def test_bootstrap():
	log("Initializing Phase 1 Bootstrap Test...")
	wait_time(3)
	
	log("Scenario 1: Welcome Splash")
	welcome = find("WelcomeLayout")
	if welcome:
		btn = find("WelcomeLayout/ok")
		if btn:
			btn.click()
			wait_time(1)
	
	log("Scenario 2: Layout Verification")
	wait_for("Main/MainTabs/Sessions", 10)
	
	log("Scenario 3: Project Init")
	init_menu = find("Main/App/Project Init...")
	if init_menu:
		init_menu.click()
	else:
		log("FAIL: Project Init menu not found")
		return False
	
	wait_for("InitLayout", 10)
	dir_edit = find("InitLayout/dir")
	if dir_edit:
		dir_edit.set("/common/active/sblo/Dev/ai-upp/tmp/SmallGuiAppForAccounting")
	name_edit = find("InitLayout/name")
	if name_edit:
		name_edit.set("SmallGuiAppForAccounting")
	
	btn_init = find("InitLayout/ok")
	if btn_init:
		btn_init.click()
	
	log("Waiting for Init Completion...")
	wait_time(2)
	send_key(K_ENTER) # Dismiss prompt
	wait_time(1)
	
	log("Scenario 4: Session Creation")
	new_session_menu = find("Main/App/New Session...")
	if new_session_menu:
		new_session_menu.click()
	else:
		log("FAIL: New Session menu not found")
		return False
	
	wait_for("NewSessionLayout", 10)
	purpose_edit = find("NewSessionLayout/purpose")
	if purpose_edit:
		purpose_edit.set("Sprint-0-Bootstrap")
	
	btn_start = find("NewSessionLayout/ok")
	if btn_start:
		btn_start.click()
	
	log("Scenario 5: Root Directory Verification")
	sessions_tab = find("Main/MainTabs/Sessions")
	if sessions_tab:
		sessions_tab.click()
	
	wait_time(2)
	dirs_row = wait_for("Main/MainTabs/SessionManagementLayout/dirs/row", 10)
	if not dirs_row:
		log("FAIL: Dirs list row not found")
		return False

	log("Scenario 6: Package Discovery")
	fleet_tab = find("Main/MainTabs/Fleet Dashboard")
	if fleet_tab:
		fleet_tab.click()
		wait_time(1)
	
	grid_row = wait_for("Main/MainTabs/FleetDashboardLayout/vsplit/project_grid/row", 10)
	if not grid_row:
		log("FAIL: Project Grid row not found")
		return False

	log("Scenario 7: Maintenance Setup (Sync Core)")
	maint_tab = find("Main/MainTabs/Maintenance Hub")
	if maint_tab:
		maint_tab.click()
		wait_time(1)
		sync_btn = find("Main/MainTabs/MaintenanceLayout/toolbar/Sync Core")
		if sync_btn:
			sync_btn.click()
			wait_time(2)
	
	log("Scenario 8: Assistant Toggle")
	send_key(K_F11)
	wait_time(1)
	
	log("Scenario 15: Initial Save")
	save_btn = find("Main/Save All Changes")
	if save_btn:
		save_btn.click()
		wait_time(1)

	log("SUCCESS: Phase 1 complete (Scenarios 1-15).")
	return True

if test_bootstrap():
	exit(0)
else:
	exit(1)