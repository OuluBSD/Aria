# AriaHub GUI Test: Threads Feed Refresh
import sys

print("Finding Threads main tab...")
threads_main_tab = find("Threads")
if not threads_main_tab:
    print("Error: Threads main tab not found")
    sys.exit(1)

print("Opening Threads...")
threads_main_tab.click()
wait_time(0.5)

print("Switching to Config tab...")
settings_tab = find("Main/Tabs/Config")
if not settings_tab:
    # Try alternate path
    settings_tab = find("Config")

if not settings_tab:
    print("Error: Config tab not found")
    print("UI Dump:")
    print(dump_ui())
    _exit(1)
settings_tab.click()
wait_time(0.2)

print("Triggering Refresh All Data...")
refresh_btn = find("Refresh All Data")
if not refresh_btn:
    print("Error: Refresh button not found")
    sys.exit(1)

# This will trigger a live scrape via ThreadsScraper
refresh_btn.click()
print("Scrape command finished. Waiting for data to populate...")
wait_time(15.0)

print("Switching back to Feed tab...")
feed_tab = find("Feed")
if feed_tab:
    feed_tab.click()
wait_time(1.0)

print("Checking for data in Feed list...")
ui = dump_ui()
if "row = " in ui:
    print("✓ Success: Feed rows detected in UI.")
else:
    print("? No feed rows detected. Scrape might have failed or returned empty.")
    # For debugging if it fails again
    # print(ui)

print("Test finished.")
_exit(0)
