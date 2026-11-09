# AriaHub GUI Test: Threads Tabs Verification
import sys

# find() in GuiAutomation searches by label/text in the UI visitor tree.

print("Finding Threads main tab...")
threads_main_tab = find("Threads")
if not threads_main_tab:
    print("Error: Threads main tab not found")
    sys.exit(1)

print("Clicking Threads main tab to expose sub-tabs...")
threads_main_tab.click()
wait_time(0.5)

print("Finding all UI elements again...")
all_els = find_all()

# Filter for the sub-tabs we want to click
targets = ["Feed", "Public Messages", "Private Messages", "Config"]
found_targets = []

for item in all_els:
    l = str(item.label)
    p = str(item.path)
    if l in targets and "Threads" in p:
        print("Found sub-tab: " + l + " at " + p)
        found_targets = found_targets + [item]

if len(found_targets) == 0:
    print("Error: No sub-tabs found after clicking main tab")
    sys.exit(1)

for item in found_targets:
    l = str(item.label)
    print("Clicking sub-tab: " + l)
    item.click()
    wait_time(0.2)

# Verify button in Settings
print("Verifying button...")
all_els = find_all()
found_btn = False
for item in all_els:
    l = str(item.label)
    if "Refresh All Data" in l:
        print("✓ Success: Verified sub-tabs and 'Refresh All Data' button.")
        found_btn = True
        break

if not found_btn:
    print("Error: Refresh button not found")
    # print(dump_ui())
    sys.exit(1)

print("Test finished successfully.")
sys.exit(0)
